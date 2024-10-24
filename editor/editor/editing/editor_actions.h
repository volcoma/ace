#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/assets/asset_handle.h>
#include <engine/ecs/prefab.h>
#include <engine/threading/threader.h>
#include <editor/deploy/deploy.h>

#include <filesystem/filesystem.h>

namespace ace
{

struct editor_actions
{
    static auto new_scene(rtti::context& ctx) -> bool;
    static auto open_scene(rtti::context& ctx) -> bool;
    static auto save_scene(rtti::context& ctx) -> bool;
    static auto save_scene_as(rtti::context& ctx) -> bool;

    static auto close_project(rtti::context& ctx) -> bool;

    static void run_project(const deploy_settings& params);
    static auto deploy_project(rtti::context& ctx, const deploy_settings& params)
        -> std::map<std::string, itc::shared_future<void>>;


    static void generate_script_workspace(const std::string& project_name);
    static void open_workspace_on_file(const std::string& project_name, const fs::path& file, int line = 0);

};

} // namespace ace
