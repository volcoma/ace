#include "script_system.h"
#include "script.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>

#include <engine/engine.h>
#include <monopp/mono_exception.h>
#include <monopp/mono_internal_call.h>
#include <monopp/mono_jit.h>
#include <monopp/mono_method_invoker.h>
#include <monort/monort.h>

#include <filesystem/filesystem.h>
#include <logging/logging.h>

#include <engine/assets/impl/asset_compiler.h>

namespace ace
{
namespace
{
std::chrono::seconds check_interval(2);

std::atomic_bool needs_recompile{};
std::mutex container_mutex;
std::set<fs::path> needs_to_recompile;

auto find_mono() -> mono::compiler_paths
{
    mono::compiler_paths result;
    {
        const auto& names = mono::get_common_library_names();
        const auto& paths = mono::get_common_library_paths();

        auto found_library = fs::find_library(names, paths);

        result.assembly_dir = fs::absolute(found_library.parent_path()).string();
        result.config_dir = fs::absolute(fs::path(result.assembly_dir) / ".." / "etc").string();
    }

    {
        const auto& names = mono::get_common_executable_names();
        const auto& paths = mono::get_common_executable_paths();

        result.msc_executable = fs::find_program(names, paths).string();
    }

    return result;
}

auto print_assembly_info(const mono::mono_assembly& assembly)
{
    std::stringstream ss;
    auto refs = assembly.dump_references();

    ss << fmt::format(" ----- References -----");

    for(const auto& ref : refs)
    {
        ss << fmt::format("\n{}", ref);
    }

    APPLOG_INFO("\n{}", ss.str());

    auto types = assembly.get_types();

    ss = {};
    ss << fmt::format(" ----- Types -----");

    for(const auto& type : types)
    {
        ss << fmt::format("\n{}", type.get_fullname());

        auto attribs = type.get_attributes();
        for(const auto& attrib : attribs)
        {
            ss << fmt::format("\n Attribute : {}", attrib.get_fullname());
        }
    }
    APPLOG_INFO("\n{}", ss.str());
}

} // namespace

auto script_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();
    ev.on_frame_update.connect(sentinel_, this, &script_system::on_frame_update);
    ev.on_play_begin.connect(sentinel_, -100, this, &script_system::on_play_begin);
    ev.on_play_end.connect(sentinel_, 100, this, &script_system::on_play_end);
    ev.on_pause.connect(sentinel_, -100, this, &script_system::on_pause);
    ev.on_resume.connect(sentinel_, 100, this, &script_system::on_resume);
    ev.on_skip_next_frame.connect(sentinel_, -100, this, &script_system::on_skip_next_frame);

    if(mono::init(find_mono(), true))
    {
        glue_.init(ctx);

        mono::mono_domain::set_assemblies_path(fs::resolve_protocol("engine:/compiled").string());

        try
        {
            load_core_domain(ctx);
        }
        catch(const mono::mono_exception& e)
        {
            APPLOG_ERROR("{}", e.what());
            return false;
        }

        return true;
    }

    return false;
}

auto script_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    glue_.deinit(ctx);

    unload_core_domain();

    mono::shutdown();

    return true;
}

void script_system::load_core_domain(rtti::context& ctx)
{
    while(true)
    {

        if(create_compilation_job(ctx, "engine").get())
        {
            break;
        }
    }

    domain_ = std::make_unique<mono::mono_domain>("Ace.Engine");
    mono::mono_domain::set_current_domain(domain_.get());

    auto engine_script_lib = fs::resolve_protocol(get_lib_compiled_key("engine"));

    auto assembly = domain_->get_assembly(engine_script_lib.string());
    print_assembly_info(assembly);
}
void script_system::unload_core_domain()
{
    domain_.reset();
    mono::mono_domain::set_current_domain(nullptr);
}

void script_system::load_app_domain(rtti::context& ctx)
{
    if(!create_compilation_job(ctx, "app").get())
    {
        // return false;
    }

    app_domain_ = std::make_unique<mono::mono_domain>("Ace.App");
    mono::mono_domain::set_current_domain(app_domain_.get());

    auto app_script_lib = fs::resolve_protocol(get_lib_compiled_key("app"));

    try
    {
        auto assembly = app_domain_->get_assembly(app_script_lib.string());
        print_assembly_info(assembly);
    }
    catch(const mono::mono_exception& e)
    {
        APPLOG_ERROR("{}", e.what());
    }
}
void script_system::unload_app_domain()
{
    app_domain_.reset();
    mono::mono_domain::set_current_domain(domain_.get());
}

void script_system::on_play_begin(rtti::context& ctx)
{
    if(!app_domain_ || !domain_)
    {
        return;
    }
    try
    {
        auto app_script_lib = fs::resolve_protocol(get_lib_compiled_key("app"));
        auto assembly = app_domain_->get_assembly(app_script_lib.string());

        auto engine_script_lib = fs::resolve_protocol(get_lib_compiled_key("engine"));
        auto engine_assembly = domain_->get_assembly(engine_script_lib.string());

        auto system_type = engine_assembly.get_type("Ace.Core", "ISystem");

        auto systems = assembly.get_types_derived_from(system_type);

        for(const auto& type : systems)
        {
            auto obj = type.new_instance();
        }
    }
    catch(const mono::mono_exception& e)
    {
        APPLOG_ERROR("{}", e.what());
    }
}

void script_system::on_play_end(rtti::context& ctx)
{
    unload_app_domain();
    load_app_domain(ctx);
}

void script_system::on_pause(rtti::context& ctx)
{
}

void script_system::on_resume(rtti::context& ctx)
{
}

void script_system::on_skip_next_frame(rtti::context& ctx)
{
}
void script_system::on_frame_update(rtti::context& ctx, delta_t dt)
{
    check_for_recompile(ctx, dt);

    if(!app_domain_ || !domain_)
    {
        return;
    }
    try
    {
        auto engine_script_lib = fs::resolve_protocol(get_lib_compiled_key("engine"));
        auto engine_assembly = domain_->get_assembly(engine_script_lib.string());

        auto system_type = engine_assembly.get_type("Ace.Core", "SystemManager");
        auto method_thunk = mono::make_method_invoker<void()>(system_type, "Update");
        method_thunk();
    }
    catch(const mono::mono_exception& e)
    {
        APPLOG_ERROR("{}", e.what());
    }
}

void script_system::check_for_recompile(rtti::context& ctx, delta_t dt)
{
    auto& ev = ctx.get<events>();
    if(ev.is_playing)
    {
        return;
    }
    time_since_last_check_ += dt;

    if(time_since_last_check_ >= check_interval)
    {
        time_since_last_check_ = {};

        bool should_recompile = needs_recompile.exchange(false);

        if(should_recompile)
        {
            auto container = []()
            {
                std::lock_guard<std::mutex> lock(container_mutex);
                auto result = std::move(needs_to_recompile);
                return result;
            }();

            for(const auto& protocol : container)
            {
                create_compilation_job(ctx, protocol)
                    .then(itc::this_thread::get_id(),
                          [this, &ctx, &ev, protocol](auto f)
                          {
                              if(ev.is_playing)
                              {
                                  return;
                              }

                              if(protocol == "app")
                              {
                                  if(f.get())
                                  {
                                      unload_app_domain();
                                      load_app_domain(ctx);
                                  }
                              }
                          });
            }
        }
    }
}

auto script_system::create_compilation_job(rtti::context& ctx, const fs::path& protocol) -> itc::job_future<bool>
{
    auto& thr = ctx.get<threader>();
    auto& am = ctx.get<asset_manager>();
    return thr.pool->schedule(
        [&am, protocol]()
        {
            auto key = get_lib_data_key(protocol).generic_string();
            auto output = get_lib_compiled_key(protocol);

            return asset_compiler::compile<script_library>(am, key, fs::resolve_protocol(output));
        });
}
void script_system::set_needs_recompile(const fs::path& protocol)
{
    needs_recompile = true;
    std::lock_guard<std::mutex> lock(container_mutex);
    needs_to_recompile.emplace(protocol);
}

auto script_system::get_lib_name(const fs::path& protocol) -> fs::path
{
    return fs::path(protocol).concat("_script.dll");
}

auto script_system::get_lib_data_key(const fs::path& protocol) -> fs::path
{
    std::string output = protocol.string() + ":/data/" + protocol.string() + "_script.dll";
    return output;
}

auto script_system::get_lib_compiled_key(const fs::path& protocol) -> fs::path
{
    std::string output = protocol.string() + ":/compiled/" + protocol.string() + "_script.dll";
    return output;
}

} // namespace ace
