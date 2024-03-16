#include "editor_actions.h"

#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/assets/impl/asset_reader.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/ecs/entity.hpp>

#include <editor/editing/editing_manager.h>
#include <editor/system/project_manager.h>

#include <filedialog/filedialog.h>
#include <filesystem/filesystem.h>
#include <subprocess/subprocess.hpp>

namespace ace
{

namespace
{

auto trim_line = [](std::string& line)
{
    // Trim trailing spaces and \r
    line.erase(std::find_if(line.rbegin(),
                            line.rend(),
                            [](char ch)
                            {
                                return !std::isspace(int(ch));
                            })
                   .base(),
               line.end());
};

auto parse_line(std::string& line, const fs::path& fs_parent_path) -> bool
{
#ifdef _WIN32
    // parse dependencies output
    if(line.find("[ApplicationDirectory]") != std::string::npos)
    {
        std::size_t pos = line.find(":");
        if(pos != std::string::npos)
        {
            line = line.substr(pos + 2); // +2 to skip ": "
            trim_line(line);

            return true;
        }
    }
#else
    // parse ldd output
    size_t pos = line.find("=> ");
    if(pos != std::string::npos)
    {
        line = line.substr(pos + 3); // +3 to remove '=> '
        size_t address_pos = line.find(" (0x");
        if(address_pos != std::string::npos)
        {
            line = line.substr(0, address_pos); // remove the address
        }

        trim_line(line);

        fs::path fs_path(line);

        if(fs::exists(fs_path) && fs::exists(fs_parent_path))
        {
            if(fs::equivalent(fs_path.parent_path(), fs_parent_path))
            {
                return true;
            }
        }
    }

#endif
    return false;
}

auto get_subprocess_params(const fs::path& file) -> std::vector<std::string>
{
    std::vector<std::string> params;

#ifdef _WIN32
    params.emplace_back(fs::resolve_protocol("editor:/programs/dependencies/Dependencies.exe"));
    params.emplace_back("-modules");
    params.emplace_back(file);

#else

    params.emplace_back("ldd");
    params.emplace_back(file);
#endif
    return params;
}

auto parse_dependencies(const std::vector<char>& input_buffer, const fs::path& fs_parent_path)
    -> std::vector<std::string>
{
    std::string input(input_buffer.begin(), input_buffer.end());

    std::vector<std::string> dependencies;
    std::stringstream ss(input);
    std::string line;

    while(std::getline(ss, line))
    {
        if(parse_line(line, fs_parent_path))
        {
            dependencies.push_back(line);
        }
    }
    return dependencies;
}

auto get_dependencies(const fs::path& file) -> std::vector<std::string>
{
    auto parent_path = file.parent_path();

    auto params = get_subprocess_params(file);
    auto obuf = subprocess::check_output(params);
    return parse_dependencies(obuf.buf, parent_path);
}

auto save_scene_impl(rtti::context& ctx, const fs::path& path) -> bool
{
    auto& ec = ctx.get<ecs>();
    save_to_file(path, ec.get_scene());
    return true;
}

auto save_scene_as_impl(rtti::context& ctx, fs::path& path) -> bool
{
    std::string picked;
    if(native::save_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Save scene as",
                                fs::resolve_protocol("app:/data").string()))
    {
        auto& em = ctx.get<editing_manager>();

        path = picked;

        if(!ex::is_format<scene_prefab>(path.extension().generic_string()))
        {
            path.replace_extension(ex::get_format<scene_prefab>(false));
        }

        return save_scene_impl(ctx, path);
    }

    return false;
}

} // namespace

auto editor_actions::new_scene(rtti::context& ctx) -> bool
{
    auto& em = ctx.get<editing_manager>();
    em.close_project();

    auto& ec = ctx.get<ecs>();
    ec.unload_scene();

    auto& def = ctx.get<defaults>();
    def.create_default_3d_scene(ctx, ec.get_scene());
    return true;
}
auto editor_actions::open_scene(rtti::context& ctx) -> bool
{
    std::string picked;
    if(native::open_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Open scene",
                                fs::resolve_protocol("app:/data").string()))
    {
        auto path = fs::convert_to_protocol(picked);
        if(ex::is_format<scene_prefab>(path.extension().generic_string()))
        {
            auto& em = ctx.get<editing_manager>();
            em.close_project();

            auto& am = ctx.get<asset_manager>();
            auto asset = am.load<scene_prefab>(path.string());

            auto& ec = ctx.get<ecs>();
            ec.unload_scene();

            auto& scene = ec.get_scene();
            return scene.load_from(asset);
        }
    }
    return false;
}
auto editor_actions::save_scene(rtti::context& ctx) -> bool
{
    auto& ec = ctx.get<ecs>();
    auto& scene = ec.get_scene();

    if(!scene.source)
    {
        fs::path picked;
        if(save_scene_as_impl(ctx, picked))
        {
            auto path = fs::convert_to_protocol(picked);

            auto& am = ctx.get<asset_manager>();
            scene.source = am.load<scene_prefab>(path.string());
            return true;
        }
    }
    else
    {
        auto path = fs::resolve_protocol(scene.source.id());
        save_scene_impl(ctx, path);
    }

    return true;
}
auto editor_actions::save_scene_as(rtti::context& ctx) -> bool
{
    fs::path p;
    return save_scene_as_impl(ctx, p);
}
auto editor_actions::close_project(rtti::context& ctx) -> bool
{
    auto& pm = ctx.get<project_manager>();
    pm.close_project(ctx);
    return true;
}


void editor_actions::run_project(const deploy_params& params)
{
    subprocess::call(params.deploy_location / ("game" + fs::executable_extension()));
}

auto editor_actions::deploy_project(rtti::context& ctx, const deploy_params& params)
    -> std::map<std::string, itc::shared_future<void>>
{
    auto& th = ctx.get<threader>();

    std::map<std::string, itc::shared_future<void>> jobs;
    std::vector<itc::shared_future<void>> jobs_seq;

    auto startup = asset_reader::resolve_compiled_path(params.startup_scene.id());

    fs::error_code ec;

    if(params.deploy_dependencies)
    {
        APPLOG_INFO("Clearing {}", params.deploy_location.string());
        fs::remove_all(params.deploy_location, ec);
        fs::create_directories(params.deploy_location, ec);

        auto job = th.pool
                       ->schedule(
                           [params]()
                           {

                               fs::path app_executable =
                                   fs::resolve_protocol("binary:/game" + fs::executable_extension());
                               auto deps = get_dependencies(app_executable);

                               fs::error_code ec;
                               for(const auto& dep : deps)
                               {
                                   APPLOG_INFO("Copying {} -> {}", dep, params.deploy_location.string());
                                   fs::copy(dep, params.deploy_location, fs::copy_options::overwrite_existing, ec);
                               }

                               APPLOG_INFO("Copying {} -> {}", app_executable.string(), params.deploy_location.string());
                               fs::copy(app_executable, params.deploy_location, fs::copy_options::overwrite_existing, ec);
                           })
                       .share();
        jobs["Deploying Dependencies"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params, startup]()
                           {
                               auto data = fs::resolve_protocol("app:/cache");
                               fs::path cached_data = params.deploy_location / "data" / "app" / "cache";
                               auto startup_path =
                                   cached_data / ("__startup__" + ex::get_format<scene_prefab>() + ".asset");

                               fs::error_code ec;

                               APPLOG_INFO("Clearing {}", cached_data.string());
                               fs::remove_all(cached_data, ec);
                               fs::create_directories(cached_data, ec);

                               APPLOG_INFO("Copying {} -> {}", data.string(), cached_data.string());
                               fs::copy(data, cached_data, fs::copy_options::recursive, ec);

                               APPLOG_INFO("Copying {} -> {}", startup.string(), cached_data.string());
                               fs::copy(startup, startup_path, fs::copy_options::overwrite_existing, ec);
                           })
                       .share();

        jobs["Deploying Project Data"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params]()
                           {
                               fs::path cached_data = params.deploy_location / "data" / "engine" / "cache";
                               auto data = fs::resolve_protocol("engine:/cache");

                               fs::error_code ec;

                               APPLOG_INFO("Clearing {}", cached_data.string());
                               fs::remove_all(cached_data, ec);
                               fs::create_directories(cached_data, ec);

                               APPLOG_INFO("Copying {} -> {}", data.string(), cached_data.string());
                               fs::copy(data, cached_data, fs::copy_options::recursive, ec);
                           })
                       .share();
        jobs["Deploying Engine Data..."] = job;
        jobs_seq.emplace_back(job);
    }

    itc::when_all(std::begin(jobs_seq), std::end(jobs_seq))
        .then(itc::this_thread::get_id(),
              [params](auto f)
              {
                  if(params.deploy_and_run)
                  {
                      run_project(params);
                  }
                  else
                  {
                      fs::show_in_graphical_env(params.deploy_location);
                  }
              });

    return jobs;
}

} // namespace ace
