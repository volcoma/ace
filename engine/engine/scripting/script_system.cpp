#include "script_system.h"
#include "script.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>

#include <engine/engine.h>
#include <monopp/mono_exception.h>
#include <monopp/mono_internal_call.h>
#include <monopp/mono_jit.h>
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

void Internal_CreateScene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
    // std::cout << "FROM C++ : MyObject created." << std::endl;
}

void Internal_DestroyScene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;
}

uint32_t Internal_CreateEntity(const std::string& tag)
{
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;

    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto e = ec.get_scene().create_entity(tag);
    return static_cast<uint32_t>(e.entity());
}

bool Internal_DestroyEntity(uint32_t id)
{
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;

    return true;
}

bool Internal_IsEntityValid(uint32_t id)
{
    return true;
}

} // namespace

auto script_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();
    ev.on_frame_update.connect(sentinel_, this, &script_system::on_frame_update);

    if(mono::init(find_mono(), true))
    {
        mono::add_internal_call("Ace.Core.Scene::Internal_CreateScene", internal_vcall(Internal_CreateScene));
        mono::add_internal_call("Ace.Core.Scene::Internal_DestroyScene", internal_vcall(Internal_DestroyScene));
        mono::add_internal_call("Ace.Core.Scene::Internal_CreateEntity", internal_rcall(Internal_CreateEntity));
        mono::add_internal_call("Ace.Core.Scene::Internal_DestroyEntity", internal_rcall(Internal_DestroyEntity));
        mono::add_internal_call("Ace.Core.Scene::Internal_IsEntityValid", internal_rcall(Internal_IsEntityValid));
        // mono::managed_interface::init(assembly);

        domain_ = std::make_unique<mono::mono_domain>("Ace.Engine");
        mono::mono_domain::set_current_domain(domain_.get());

        try
        {
            if(!create_compilation_job(ctx, "engine").get())
            {
                // return false;
            }

            // auto assembly = domain_->get_assembly(fs::resolve_protocol("engine:/compiled/engine_script.dll").string());
            // auto refs = assembly.dump_references();

            // APPLOG_INFO("\nAssembly References for {} :", "engine_script.dll");

            // for(const auto& ref : refs)
            // {
            //     APPLOG_INFO("\n{}", ref);
            // }

            // auto type = assembly.get_type("Ace.Core", "Scene");
            // auto obj = type.new_instance();
        }
        catch(const std::exception& e)
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

    mono::mono_domain::set_current_domain(nullptr);
    domain_.reset();

    mono::shutdown();

    return true;
}

void script_system::on_frame_update(rtti::context& ctx, delta_t dt)
{
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
                create_compilation_job(ctx, protocol);
            }
        }
    }

    if(os::key::is_pressed(os::key::t))
    {
        auto assembly = domain_->get_assembly(fs::resolve_protocol("engine:/compiled/engine_script.dll").string());
        auto type = assembly.get_type("Ace.Core", "Scene");
        auto obj = type.new_instance();
    }
}
auto script_system::create_compilation_job(rtti::context& ctx, const fs::path& protocol) -> itc::job_future<bool>
{
    auto& thr = ctx.get<threader>();
    auto& am = ctx.get<asset_manager>();
    return thr.pool->schedule(
        [&am, protocol]()
        {
            std::string key = protocol.string() + ":/data/" + protocol.string() + "_script.dll";
            std::string output = protocol.string() + ":/compiled/" + protocol.string() + "_script.dll";

            return asset_compiler::compile<script_library>(am, key, fs::resolve_protocol(output));
        });
}
void script_system::set_needs_recompile(const fs::path& protocol)
{
    needs_recompile = true;
    std::lock_guard<std::mutex> lock(container_mutex);
    needs_to_recompile.emplace(protocol);
}

} // namespace ace
