#pragma once
#include <engine/engine_export.h>

#include <base/basetypes.hpp>
#include <engine/assets/asset_handle.h>
#include <engine/ecs/scene.h>
#include <string>


namespace ace
{

struct settings
{
    struct app_settings
    {
        std::string company;
        std::string product;
        std::string version;
    } app;

    struct graphics_settings
    {
    } graphics;

    struct standalone_settings
    {
        asset_handle<scene_prefab> startup_scene;
    } standalone;

};
} // namespace ace
