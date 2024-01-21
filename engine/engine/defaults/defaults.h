#pragma once

#include "engine/rendering/camera.h"
#include "engine/rendering/reflection_probe.h"
#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <engine/ecs/ecs.h>
#include <engine/rendering/light.h>
#include <engine/rendering/reflection_probe.h>

namespace ace
{

struct defaults
{
    defaults();
    ~defaults();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    auto init_assets(rtti::context& ctx) -> bool;

    auto create_embedded_mesh_entity(rtti::context& ctx, scene& scn, const std::string& name) -> entt::handle;

    auto create_prefab_at(rtti::context& ctx, scene& scn, const std::string& key, const camera& cam, math::vec2 pos)
        -> entt::handle;

    auto create_mesh_entity_at(rtti::context& ctx,
                               scene& scn,
                               const std::string& key,
                               const camera& cam,
                               math::vec2 pos) -> entt::handle;

    auto create_light_entity(rtti::context& ctx, scene& scn, light_type type, const std::string& name) -> entt::handle;

    auto create_reflection_probe_entity(rtti::context& ctx, scene& scn, probe_type type, const std::string& name)
        -> entt::handle;
    auto create_camera_entity(rtti::context& ctx, scene& scn, const std::string& name) -> entt::handle;

    void create_default_3d_scene(rtti::context& ctx, scene& scn);

    void focus_camera_on_entity(entt::handle camera, entt::handle entity);
};
} // namespace ace
