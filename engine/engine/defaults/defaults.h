#pragma once

#include "engine/rendering/camera.h"
#include "engine/rendering/reflection_probe.h"
#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <engine/ecs/ecs.h>
#include <engine/rendering/light.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/rendering/reflection_probe.h>
namespace ace
{

struct defaults
{
    static auto init(rtti::context& ctx) -> bool;
    static auto deinit(rtti::context& ctx) -> bool;

    static auto init_assets(rtti::context& ctx) -> bool;

    static auto create_embedded_mesh_entity(rtti::context& ctx, scene& scn, const std::string& name) -> entt::handle;

    static auto create_prefab_at(rtti::context& ctx,
                                 scene& scn,
                                 const std::string& key,
                                 const camera& cam,
                                 math::vec2 pos) -> entt::handle;

    static auto create_prefab_at(rtti::context& ctx,
                                 scene& scn,
                                 const std::string& key,
                                 math::vec3 pos = {0.0f, 0.0f, 0.0f}) -> entt::handle;

    static auto create_mesh_entity_at(rtti::context& ctx,
                                      scene& scn,
                                      const std::string& key,
                                      const camera& cam,
                                      math::vec2 pos) -> entt::handle;

    static auto create_mesh_entity_at(rtti::context& ctx,
                                      scene& scn,
                                      const std::string& key,
                                      math::vec3 pos = {0.0f, 0.0f, 0.0f}) -> entt::handle;

    static auto create_light_entity(rtti::context& ctx, scene& scn, light_type type, const std::string& name)
        -> entt::handle;

    static auto create_reflection_probe_entity(rtti::context& ctx, scene& scn, probe_type type, const std::string& name)
        -> entt::handle;
    static auto create_camera_entity(rtti::context& ctx, scene& scn, const std::string& name) -> entt::handle;

    static void create_default_3d_scene(rtti::context& ctx, scene& scn);

    static void focus_camera_on_entity(entt::handle camera, entt::handle entity);

    template<typename T>
    static void create_default_3d_scene_for_asset_preview(rtti::context& ctx,
                                                          scene& scn,
                                                          const asset_handle<T>& asset,
                                                          const usize32_t& size);

private:
    static auto create_default_3d_scene_for_preview(rtti::context& ctx, scene& scn, const usize32_t& size)
        -> entt::handle;
};
} // namespace ace
