#include "project_manager.h"
#include <editor/editing/editing_manager.h>
#include <editor/editing/thumbnail_manager.h>
#include <editor/meta/system/project_manager.hpp>
#include <editor/assets/asset_watcher.h>

#include <engine/meta/settings/settings.hpp>
#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_compiler.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/threading/threader.h>
#include <engine/ecs/ecs.h>

#include <filesystem/watcher.h>
#include <graphics/graphics.h>
#include <logging/logging.h>
#include <serialization/associative_archive.h>
#include <serialization/serialization.h>

#include <fstream>

namespace ace
{

void project_manager::close_project(rtti::context& ctx)
{
    if(has_open_project())
    {
        save_project_settings();
        project_settings_ = {};
    }

    auto& em = ctx.get<editing_manager>();
    em.close_project();

    auto& tm = ctx.get<thumbnail_manager>();
    tm.clear_thumbnails();

    auto& ec = ctx.get<ecs>();
    ec.unload_scene();

    set_name({});

    auto& aw = ctx.get<asset_watcher>();
    aw.unwatch_assets(ctx, "app:/");


    load_config();
}

auto project_manager::open_project(rtti::context& ctx, const fs::path& project_path) -> bool
{
    close_project(ctx);

    {
        fs::error_code err;
        fs::create_directories(fs::resolve_protocol("app:/data"), err);
        fs::create_directories(fs::resolve_protocol("app:/compiled"), err);
        fs::create_directories(fs::resolve_protocol("app:/meta"), err);
        fs::create_directories(fs::resolve_protocol("app:/settings"), err);
    }


    fs::error_code err;
    if(!fs::exists(project_path, err))
    {
        APPLOG_ERROR("Project directory doesn't exist {0}", project_path.string());
        return false;
    }

    fs::add_path_protocol("app", project_path);

    set_name(project_path.filename().string());

    load_project_settings();
    save_project_settings();

    save_config();

    auto& aw = ctx.get<asset_watcher>();
    aw.watch_assets(ctx, "app:/");

    return true;
}


void project_manager::load_project_settings()
{
    load_from_file(fs::resolve_protocol("app:/settings/settings.cfg").string(), project_settings_);
}

void project_manager::save_project_settings()
{
    save_to_file(fs::resolve_protocol("app:/settings/settings.cfg").string(), project_settings_);
}

void project_manager::create_project(rtti::context& ctx, const fs::path& project_path)
{
    fs::error_code err;
    fs::add_path_protocol("app", project_path);


    open_project(ctx, project_path);
}

void project_manager::load_config()
{
    fs::error_code err;
    const fs::path project_config_file = fs::resolve_protocol("editor:/config/project.cfg");
    if(!fs::exists(project_config_file, err))
    {
        save_config();
    }
    else
    {
        std::ifstream output(project_config_file.string());
        ser20::iarchive_associative_t ar(output);

        try_load(ar, ser20::make_nvp("options", options_));

        auto& items = options_.recent_projects;
        auto iter = std::begin(items);
        while(iter != items.end())
        {
            auto& item = *iter;

            if(!fs::exists(item, err))
            {
                iter = items.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
}

auto project_manager::get_name() const -> const std::string&
{
    return project_name_;
}

void project_manager::set_name(const std::string& name)
{
    project_name_ = name;
}

auto project_manager::get_settings() -> settings&
{
    return project_settings_;
}

auto project_manager::get_options() -> options&
{
    return options_;
}

auto project_manager::has_open_project() const -> bool
{
    return !get_name().empty();
}

void project_manager::save_config()
{
    if(has_open_project())
    {
        auto& rp = options_.recent_projects;
        auto project_path = fs::resolve_protocol("app:/");
        if(std::find(std::begin(rp), std::end(rp), project_path.generic_string()) == std::end(rp))
        {
            rp.push_back(project_path.generic_string());
        }
    }

    fs::error_code err;
    fs::create_directory(fs::resolve_protocol("editor:/config"), err);
    const auto project_config_file = fs::resolve_protocol("editor:/config/project.cfg").string();
    std::ofstream output(project_config_file);
    ser20::oarchive_associative_t ar(output);

    try_save(ar, ser20::make_nvp("options", options_));
}

project_manager::project_manager()
{
    load_config();
}

auto project_manager::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& aw = ctx.get<asset_watcher>();
    aw.watch_assets(ctx, "editor:/", true);

    return true;
}

auto project_manager::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    close_project(ctx);

    auto& aw = ctx.get<asset_watcher>();
    aw.unwatch_assets(ctx, "editor:/");

    return true;
}

project_manager::~project_manager()
{
    save_config();
}
} // namespace ace
