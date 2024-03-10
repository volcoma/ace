#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/assets/asset_handle.h>
#include <engine/ecs/prefab.h>
#include <engine/threading/threader.h>

#include <filesystem/filesystem.h>

namespace ace
{
struct deploy_params
{
    asset_handle<scene_prefab> startup_scene;
    fs::path deploy_location{};
    bool deploy_dependencies{true};
    bool deploy_and_run{};
};

struct editor_actions
{
    static auto new_scene(rtti::context& ctx) -> bool;
    static auto open_scene(rtti::context& ctx) -> bool;
    static auto save_scene(rtti::context& ctx) -> bool;
    static auto save_scene_as(rtti::context& ctx) -> bool;

    static auto close_project(rtti::context& ctx) -> bool;

    static auto deploy_project(rtti::context& ctx, const deploy_params& params)
        -> std::map<std::string, itc::job_shared_future<void>>;
};

} // namespace ace
