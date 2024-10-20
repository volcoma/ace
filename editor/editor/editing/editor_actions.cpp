#include "editor_actions.h"

#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/assets/impl/asset_reader.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/assets/asset_database.hpp>
#include <engine/meta/ecs/entity.hpp>

#include <editor/editing/editing_manager.h>
#include <editor/system/project_manager.h>

#include <filedialog/filedialog.h>
#include <filesystem/filesystem.h>
#include <subprocess/subprocess.hpp>

#include <base/platform/config.hpp>

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
#ifdef ACE_PLATFORM_WINDOWS
    // parse dependencies output
    if(line.find("[ApplicationDirectory]") != std::string::npos)
    {
        std::size_t pos = line.find(':');
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

#ifdef ACE_PLATFORM_WINDOWS
    params.emplace_back(fs::resolve_protocol("editor:/tools/dependencies/Dependencies.exe").string());
    params.emplace_back("-modules");
    params.emplace_back(file.string());

#else

    params.emplace_back("ldd");
    params.emplace_back(file.string());
#endif
    return params;
}

auto parse_dependencies(const std::string& input, const fs::path& fs_parent_path)
    -> std::vector<std::string>
{
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
    auto result = subprocess::call(params);
    return parse_dependencies(result.out_output, parent_path);
}

auto save_scene_impl(rtti::context& ctx, const fs::path& path) -> bool
{
    auto& ec = ctx.get<ecs>();
    save_to_file(path.string(), ec.get_scene());

    return true;
}

auto save_scene_as_impl(rtti::context& ctx, fs::path& path) -> bool
{
    std::string picked;
    if(native::save_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Save scene as",
                                fs::resolve_protocol("app:/data/").string()))
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

    defaults::create_default_3d_scene(ctx, ec.get_scene());
    return true;
}
auto editor_actions::open_scene(rtti::context& ctx) -> bool
{
    std::string picked;
    if(native::open_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Open scene",
                                fs::resolve_protocol("app:/data/").string()))
    {
        auto path = fs::convert_to_protocol(picked);
        if(ex::is_format<scene_prefab>(path.extension().generic_string()))
        {
            auto& em = ctx.get<editing_manager>();
            em.close_project();

            auto& am = ctx.get<asset_manager>();
            auto asset = am.get_asset<scene_prefab>(path.string());

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
            scene.source = am.get_asset<scene_prefab>(path.string());
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

void editor_actions::run_project(const deploy_settings& params)
{
    auto call_params = params.deploy_location / (std::string("game") + fs::executable_extension());
    subprocess::call(call_params.string());
}

auto editor_actions::deploy_project(rtti::context& ctx, const deploy_settings& params)
    -> std::map<std::string, itc::shared_future<void>>
{
    auto& th = ctx.get<threader>();

    std::map<std::string, itc::shared_future<void>> jobs;
    std::vector<itc::shared_future<void>> jobs_seq;

    fs::error_code ec;

    auto& am = ctx.get<asset_manager>();
    // am.get_database("engine:/")

    if(params.deploy_dependencies)
    {
        APPLOG_INFO("Clearing {}", params.deploy_location.string());
        fs::remove_all(params.deploy_location, ec);
        fs::create_directories(params.deploy_location, ec);

        auto job =
            th.pool
                ->schedule(
                    [params]()
                    {
                        APPLOG_INFO("Deploying Dependencies...");

                        fs::path app_executable = fs::resolve_protocol("binary:/game" + fs::executable_extension());
                        auto deps = get_dependencies(app_executable);

                        fs::error_code ec;
                        for(const auto& dep : deps)
                        {
                            APPLOG_TRACE("Copying {} -> {}", dep, params.deploy_location.string());
                            fs::copy(dep, params.deploy_location, fs::copy_options::overwrite_existing, ec);
                        }

                        APPLOG_TRACE("Copying {} -> {}", app_executable.string(), params.deploy_location.string());
                        fs::copy(app_executable, params.deploy_location, fs::copy_options::overwrite_existing, ec);

                        APPLOG_INFO("Deploying Dependencies - Done...");

                    })
                .share();
        jobs["Deploying Dependencies"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params]()
                           {
                               APPLOG_INFO("Deploying Project Settings...");

                               auto data = fs::resolve_protocol("app:/settings");
                               fs::path dst = params.deploy_location / "data" / "app" / "settings";

                               fs::error_code ec;

                               APPLOG_TRACE("Clearing {}", dst.string());
                               fs::remove_all(dst, ec);
                               fs::create_directories(dst, ec);

                               APPLOG_TRACE("Copying {} -> {}", data.string(), dst.string());
                               fs::copy(data, dst, fs::copy_options::recursive, ec);

                               APPLOG_INFO("Deploying Project Settings - Done...");

                           })
                       .share();

        jobs["Deploying Project Settings"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params, &am]()
                           {
                               APPLOG_INFO("Deploying Project Data...");

                               fs::error_code ec;
                               {
                                   auto data = fs::resolve_protocol("app:/compiled");
                                   fs::path cached_data = params.deploy_location / "data" / "app" / "compiled";

                                   APPLOG_TRACE("Clearing {}", cached_data.string());
                                   fs::remove_all(cached_data, ec);
                                   fs::create_directories(cached_data, ec);

                                   APPLOG_TRACE("Copying {} -> {}", data.string(), cached_data.string());
                                   fs::copy(data, cached_data, fs::copy_options::recursive, ec);
                               }

                               {
                                   fs::path cached_data = params.deploy_location / "data" / "app" / "assets.pack";
                                   APPLOG_TRACE("Creating Asset Pack -> {}", cached_data.string());
                                   am.save_database("app:/", cached_data);
                               }

                               APPLOG_INFO("Deploying Project Data - Done...");

                           })
                       .share();

        jobs["Deploying Project Data"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params, &am]()
                           {
                               APPLOG_INFO("Deploying Engine Data...");

                               fs::error_code ec;
                               {
                                   fs::path cached_data = params.deploy_location / "data" / "engine" / "compiled";
                                   auto data = fs::resolve_protocol("engine:/compiled");

                                   APPLOG_TRACE("Clearing {}", cached_data.string());
                                   fs::remove_all(cached_data, ec);
                                   fs::create_directories(cached_data, ec);

                                   APPLOG_TRACE("Copying {} -> {}", data.string(), cached_data.string());
                                   fs::copy(data, cached_data, fs::copy_options::recursive, ec);
                               }

                               {
                                   fs::path cached_data = params.deploy_location / "data" / "engine" / "assets.pack";
                                   APPLOG_TRACE("Creating Asset Pack -> {}", cached_data.string());
                                   am.save_database("engine:/", cached_data);
                               }

                               APPLOG_INFO("Deploying Engine Data - Done...");

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
