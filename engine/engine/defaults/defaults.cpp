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

namespace
{
void process_node(const std::unique_ptr<mesh::armature_node>& node,
                  const skin_bind_data& bind_data,
                  entt::handle parent,
                  std::vector<entt::handle>& entity_nodes,
                  scene& scn)
{
    if(!parent)
        return;

    auto entity_node = parent;

    auto bone = bind_data.find_bone_by_id(node->name);
    if(bone)
    {
        entity_node = scn.create_entity(node->name, parent);

        auto& trans_comp = entity_node.get<transform_component>();
        trans_comp.set_transform_local(node->local_transform);

        entity_nodes.push_back(entity_node);
    }

    for(auto& child : node->children)
    {
        process_node(child, bind_data, entity_node, entity_nodes, scn);
    }
}


math::bbox calc_bounds(entt::handle entity)
{
    const math::vec3 one = {1.0f, 1.0f, 1.0f};
    math::bbox bounds = math::bbox(-one, one);
    auto& ent_trans_comp = entity.get<transform_component>();
    {
        auto target_pos = ent_trans_comp.get_position_global();
        bounds = math::bbox(target_pos - one, target_pos + one);

        auto ent_model_comp = entity.try_get<model_component>();
        if(ent_model_comp)
        {
            const auto& model = ent_model_comp->get_model();
            if(model.is_valid())
            {
                const auto lod = model.get_lod(0);
                if(lod)
                {
                    const auto& mesh = lod.get();
                    bounds = mesh.get_bounds();
                }
            }
        }
        const auto& world = ent_trans_comp.get_transform_global();
        bounds = math::bbox::mul(bounds, world);
    }
    return bounds;
};

void focus_camera_on_bounds(entt::handle camera, const math::bbox& bounds)
{
    auto& trans_comp = camera.get<transform_component>();
    auto& camera_comp = camera.get<camera_component>();
    const auto& cam = camera_comp.get_camera();

    math::vec3 cen = bounds.get_center();
    math::vec3 size = bounds.get_dimensions();

    float aspect = cam.get_aspect_ratio();
    float fov = cam.get_fov();
    // Get the radius of a sphere circumscribing the bounds
    float radius = math::length(size) / 2.0f;
    // Get the horizontal FOV, since it may be the limiting of the two FOVs to properly
    // encapsulate the objects
    float horizontalFOV = math::degrees(2.0f * math::atan(math::tan(math::radians(fov) / 2.0f) * aspect));
    // Use the smaller FOV as it limits what would get cut off by the frustum
    float mfov = math::min(fov, horizontalFOV);
    float dist = radius / (math::sin(math::radians(mfov) / 2.0f));

    camera_comp.set_ortho_size(radius);
    trans_comp.set_position_global(cen - dist * trans_comp.get_z_axis_global());
    trans_comp.look_at(cen);
}
} // namespace
defaults::defaults()
{
}

defaults::~defaults()
{
    material::default_color_map() = {};
    material::default_normal_map() = {};
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
    }
    {
        const auto id = "embedded:/standard";
        auto instance = std::make_shared<pbr_material>();
        auto asset = manager.load_asset_from_instance<material>(id, instance);

        model::default_material() = asset;
    }

    {
        const auto id = "embedded:/fallback";
        auto instance = std::make_shared<pbr_material>();
        instance->set_emissive_color(math::color::purple());
        instance->set_roughness(1.0f);
        auto asset = manager.load_asset_from_instance<material>(id, instance);

        model::fallback_material() = asset;
    }

    return true;
}

auto defaults::create_embedded_mesh_entity(rtti::context& ctx, scene& scn, const std::string& name) -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    const auto id = "embedded:/" + string_utils::to_lower(name);

    model model;
    model.set_lod(am.load<mesh>(id), 0);
    model.set_material(am.load<material>("embedded:/standard"), 0);
    auto object = scn.create_entity();
    object.get_or_emplace<tag_component>().tag = name;

    auto& transf_comp = object.get_or_emplace<transform_component>();
    transf_comp.set_position_local({0.0f, 0.5f, 0.0f});

    auto& model_comp = object.get_or_emplace<model_component>();
    model_comp.set_casts_shadow(true);
    model_comp.set_casts_reflection(false);
    model_comp.set_model(model);

    return object;
}

auto defaults::create_prefab_at(rtti::context& ctx, scene& scn, const std::string& key, math::vec3 pos)
    -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto asset = am.load<prefab>(key);

    auto object = scn.instantiate(asset);

    auto& trans_comp = object.get<transform_component>();
    trans_comp.set_position_global(pos);


    return object;
}

auto defaults::create_prefab_at(rtti::context& ctx, scene& scn, const std::string& key, const camera& cam, math::vec2 pos)
    -> entt::handle
{
    math::vec3 projected_pos{0.0f, 0.0f, 0.0f};
    cam.viewport_to_world(pos,
                             math::plane::from_point_normal(math::vec3{0.0f, 0.0f, 0.0f}, math::vec3{0.0f, 1.0f, 0.0f}),
                             projected_pos,
                             false);

    return create_prefab_at(ctx, scn, key, projected_pos);
}

auto defaults::create_mesh_entity_at(rtti::context& ctx, scene& scn, const std::string& key, math::vec3 pos)
    -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto asset = am.load<mesh>(key);

    model mdl;
    mdl.set_lod(asset, 0);

    auto lod = mdl.get_lod(0);
    // If mesh isnt loaded yet skip it.
    if(!lod)
        return {};

    std::string name = fs::path(key).stem().string();
    auto object = scn.create_entity(name);
    // Add component and configure it.


    auto& trans_comp = object.get<transform_component>();

    // Add component and configure it.
    auto& model_comp = object.emplace<model_component>();
    model_comp.set_casts_shadow(true);
    model_comp.set_casts_reflection(false);
    model_comp.set_model(mdl);

    const auto& mesh = lod.get();

    const auto& skin_data = mesh.get_skin_bind_data();

    // Has skinning data?
    if(skin_data.has_bones())
    {
        const auto& armature = mesh.get_armature();
        trans_comp.set_transform_local(armature->local_transform);


        std::vector<entt::handle> bone_entities;
        process_node(armature, skin_data, object, bone_entities, scn);
        model_comp.set_bone_entities(bone_entities);
        model_comp.set_static(false);
    }

    trans_comp.set_position_global(pos);

    return object;
}

auto defaults::create_mesh_entity_at(rtti::context& ctx, scene& scn, const std::string& key, const camera& cam, math::vec2 pos)
    -> entt::handle
{
    math::vec3 projected_pos{0.0f, 0.0f, 0.0f};
    bool projected = cam.viewport_to_world(pos,
                             math::plane::from_point_normal(math::vec3{0.0f, 0.0f, 0.0f}, math::vec3{0.0f, 1.0f, 0.0f}),
                             projected_pos,
                                           false);

    return create_mesh_entity_at(ctx, scn, key, projected_pos);
}

auto defaults::create_light_entity(rtti::context& ctx, scene& scn, light_type type, const std::string& name) -> entt::handle
{
    auto& am = ctx.get<asset_manager>();

    auto object = scn.create_entity();
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

auto defaults::create_reflection_probe_entity(rtti::context& ctx, scene& scn, probe_type type, const std::string& name)
    -> entt::handle
{
    auto& am = ctx.get<asset_manager>();

    auto object = scn.create_entity();
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

auto defaults::create_camera_entity(rtti::context& ctx, scene& scn, const std::string& name) -> entt::handle
{
    auto object = scn.create_entity();
    object.get_or_emplace<tag_component>().tag = name;

    auto& transf_comp = object.get_or_emplace<transform_component>();
    transf_comp.set_position_local({0.0f, 1.0f, -10.0f});

    object.emplace<camera_component>();

    return object;
}

void defaults::create_default_3d_scene(rtti::context& ctx, scene& scn)
{
    create_camera_entity(ctx, scn, "Main Camera");

    {
        auto object = create_light_entity(ctx, scn, light_type::directional, "Sky & Directional");
        object.emplace<skylight_component>();
    }

    {
        auto object = create_reflection_probe_entity(ctx, scn, probe_type::sphere, "Envinroment");
        auto& reflection_comp = object.get_or_emplace<reflection_probe_component>();
        auto probe = reflection_comp.get_probe();
        probe.method = reflect_method::environment;
        probe.sphere_data.range = 1000.0f;
        reflection_comp.set_probe(probe);
    }
}

void defaults::focus_camera_on_entity(entt::handle camera, entt::handle entity)
{
    if(camera.all_of<transform_component, camera_component>())
    {
        auto bounds = calc_bounds(entity);
        focus_camera_on_bounds(camera, bounds);
    }
}
} // namespace ace
