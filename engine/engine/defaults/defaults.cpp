#include "defaults.h"

#include <engine/assets/asset_manager.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>

#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/id_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/transform_component.h>

#include <logging/logging.h>
#include <string_utils/utils.h>

namespace ace
{
defaults::defaults()
{
}

defaults::~defaults()
{
    material::default_color_map() = {};
    material::default_normal_map() = {};

    standard_material::program() = {};
    standard_material::program_skinned() = {};
}

auto defaults::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return init_assets(ctx);
}

auto defaults::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto defaults::init_assets(rtti::context& ctx) -> bool
{
    auto& manager = ctx.get<asset_manager>();
    {
        const auto id = "embedded:/cube";
        auto instance = std::make_shared<mesh>();
        instance->create_cube(gfx::mesh_vertex::get_layout(), 1.0f, 1.0f, 1.0f, 1, 1, 1, mesh_create_origin::center);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/sphere";
        auto instance = std::make_shared<mesh>();
        instance->create_sphere(gfx::mesh_vertex::get_layout(), 0.5f, 20, 20, mesh_create_origin::center);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/plane";
        auto instance = std::make_shared<mesh>();
        instance->create_plane(gfx::mesh_vertex::get_layout(), 10.0f, 10.0f, 1, 1, mesh_create_origin::center);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/cylinder";
        auto instance = std::make_shared<mesh>();
        instance->create_cylinder(gfx::mesh_vertex::get_layout(), 0.5f, 2.0f, 20, 20, mesh_create_origin::center);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/capsule";
        auto instance = std::make_shared<mesh>();
        instance->create_capsule(gfx::mesh_vertex::get_layout(), 0.5f, 2.0f, 20, 20, mesh_create_origin::center);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/cone";
        auto instance = std::make_shared<mesh>();
        instance->create_cone(gfx::mesh_vertex::get_layout(), 0.5f, 0.0f, 2, 20, 20, mesh_create_origin::bottom);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/torus";
        auto instance = std::make_shared<mesh>();
        instance->create_torus(gfx::mesh_vertex::get_layout(), 1.0f, 0.5f, 20, 20, mesh_create_origin::center);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/teapot";
        auto instance = std::make_shared<mesh>();
        instance->create_teapot(gfx::mesh_vertex::get_layout());
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/icosahedron";
        auto instance = std::make_shared<mesh>();
        instance->create_icosahedron(gfx::mesh_vertex::get_layout());
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/dodecahedron";
        auto instance = std::make_shared<mesh>();
        instance->create_dodecahedron(gfx::mesh_vertex::get_layout());
        manager.load_asset_from_instance(id, instance);
    }

    for(int i = 0; i < 20; ++i)
    {
        const auto id = std::string("embedded:/icosphere") + std::to_string(i);
        auto instance = std::make_shared<mesh>();
        instance->create_icosphere(gfx::mesh_vertex::get_layout(), i);
        manager.load_asset_from_instance(id, instance);
    }

    {
        material::default_color_map() = manager.load<gfx::texture>("engine:/data/textures/default_color.dds");
        material::default_normal_map() = manager.load<gfx::texture>("engine:/data/textures/default_normal.dds");

        auto vs_deferred_geom = manager.load<gfx::shader>("engine:/data/shaders/vs_deferred_geom.sc");
        auto vs_deferred_geom_skinned = manager.load<gfx::shader>("engine:/data/shaders/vs_deferred_geom_skinned.sc");
        auto fs_deferred_geom = manager.load<gfx::shader>("engine:/data/shaders/fs_deferred_geom.sc");

        standard_material::program() = gpu_program(vs_deferred_geom, fs_deferred_geom);
        standard_material::program_skinned() = gpu_program(vs_deferred_geom_skinned, fs_deferred_geom);
    }
    {
        const auto id = "embedded:/standard";
        auto instance = std::make_shared<standard_material>();
        auto asset = manager.load_asset_from_instance<material>(id, instance);

        model::default_material() = asset;
    }

    {
        const auto id = "embedded:/fallback";
        auto instance = std::make_shared<standard_material>();
        instance->set_emissive_color(math::color::purple());
        instance->set_roughness(1.0f);
        auto asset = manager.load_asset_from_instance<material>(id, instance);

        model::fallback_material() = asset;
    }

    return true;
}

auto defaults::create_embedded_mesh_entity(rtti::context& ctx, const std::string& name) -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto& ec = ctx.get<ecs>();
    const auto id = "embedded:/" + string_utils::to_lower(name);

    model model;
    model.set_lod(am.load<mesh>(id), 0);
    model.set_material(am.load<material>("embedded:/standard"), 0);
    auto object = ec.create_entity();
    object.get_or_emplace<tag_component>().tag = name;

    auto& transf_comp = object.get_or_emplace<transform_component>();
    transf_comp.set_position_local({0.0f, 0.5f, 0.0f});

    auto& model_comp = object.get_or_emplace<model_component>();
    model_comp.set_casts_shadow(true);
    model_comp.set_casts_reflection(false);
    model_comp.set_model(model);

    return object;
}

auto defaults::create_prefab_at(rtti::context& ctx, const std::string& key, const camera& cam, math::vec2 pos)
    -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto& ec = ctx.get<ecs>();
    auto asset = am.load<prefab>(key);

    auto object = ec.instantiate(asset);

    math::vec3 projected_pos;
    if(cam.viewport_to_world(pos,
                             math::plane::from_point_normal(math::vec3{0.0f, 0.0f, 0.0f}, math::vec3{0.0f, 1.0f, 0.0f}),
                             projected_pos,
                             false))
    {
        auto& trans_comp = object.get<transform_component>();
        trans_comp.set_position_global(projected_pos);
    }

    return object;
}

auto defaults::create_mesh_entity_at(rtti::context& ctx, const std::string& key, const camera& cam, math::vec2 pos)
    -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto& ec = ctx.get<ecs>();
    auto asset = am.load<mesh>(key);

    model mdl;
    mdl.set_lod(asset, 0);

    auto object = ec.create_entity();
    // Add component and configure it.

    math::vec3 projected_pos;
    if(cam.viewport_to_world(pos,
                             math::plane::from_point_normal(math::vec3{0.0f, 0.0f, 0.0f}, math::vec3{0.0f, 1.0f, 0.0f}),
                             projected_pos,
                             false))
    {
        auto& trans_comp = object.get<transform_component>();
        trans_comp.set_position_global(projected_pos);
    }

    // Add component and configure it.
    auto& model_comp = object.emplace<model_component>();
    model_comp.set_casts_shadow(true);
    model_comp.set_casts_reflection(false);
    model_comp.set_model(mdl);

    return object;
}

auto defaults::create_light_entity(rtti::context& ctx, light_type type, const std::string& name) -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto& ec = ctx.get<ecs>();

    auto object = ec.create_entity();
    object.get_or_emplace<tag_component>().tag = name + " Light";

    auto& transf_comp = object.get_or_emplace<transform_component>();
    transf_comp.set_position_local({0.0f, 1.0f, 0.0f});
    transf_comp.rotate_by_euler_local({50.0f, -30.0f, 0.0f});

    light light_data;
    light_data.color = math::color(255, 244, 214, 255);
    light_data.type = type;

    auto& light_comp = object.get_or_emplace<light_component>();
    light_comp.set_light(light_data);

    return object;
}

auto defaults::create_reflection_probe_entity(rtti::context& ctx, probe_type type, const std::string& name)
    -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto& ec = ctx.get<ecs>();

    auto object = ec.create_entity();
    object.get_or_emplace<tag_component>().tag = name + " Probe";

    auto& transf_comp = object.get_or_emplace<transform_component>();
    transf_comp.set_position_local({0.0f, 0.1f, 0.0f});

    reflection_probe probe;
    probe.method = reflect_method::static_only;
    probe.type = type;

    auto& reflection_comp = object.get_or_emplace<reflection_probe_component>();
    reflection_comp.set_probe(probe);

    return object;
}

auto defaults::create_camera_entity(rtti::context& ctx, const std::string& name) -> entt::handle
{
    auto& ec = ctx.get<ecs>();

    auto object = ec.create_entity();
    object.get_or_emplace<tag_component>().tag = name;

    auto& transf_comp = object.get_or_emplace<transform_component>();
    transf_comp.set_position_local({0.0f, 1.0f, -10.0f});

    object.emplace<camera_component>();

    return object;
}

void defaults::create_default_3d_scene(rtti::context& ctx)
{
    create_camera_entity(ctx, "Main Camera");
    create_light_entity(ctx, light_type::directional, "Directional");
}
} // namespace ace
