#include "defaults.h"

#include <engine/assets/asset_manager.h>

#include <engine/audio/ecs/components/audio_listener_component.h>
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

void focus_camera_on_bounds(entt::handle camera, const math::bsphere& bounds)
{
    auto& trans_comp = camera.get<transform_component>();
    auto& camera_comp = camera.get<camera_component>();
    const auto& cam = camera_comp.get_camera();

    math::vec3 cen = bounds.position;

    float aspect = cam.get_aspect_ratio();
    float fov = cam.get_fov();
    // Get the radius of a sphere circumscribing the bounds
    float radius = bounds.radius;
    // Get the horizontal FOV, since it may be the limiting of the two FOVs to properly
    // encapsulate the objects
    float horizontalFOV = math::degrees(2.0f * math::atan(math::tan(math::radians(fov) / 2.0f) * aspect));
    // Use the smaller FOV as it limits what would get cut off by the frustum
    float mfov = math::min(fov, horizontalFOV);
    float dist = radius / (math::sin(math::radians(mfov) / 2.0f));

    trans_comp.look_at(cen);
    trans_comp.set_position_global(cen - dist * trans_comp.get_z_axis_global());
    camera_comp.set_ortho_size(radius);
    camera_comp.update(trans_comp.get_transform_global());

}

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

    trans_comp.look_at(cen);
    trans_comp.set_position_global(cen - dist * trans_comp.get_z_axis_global());
    camera_comp.set_ortho_size(radius);
    camera_comp.update(trans_comp.get_transform_global());
}
} // namespace

auto defaults::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str<defaults>(), __func__);

    return init_assets(ctx);
}

auto defaults::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str<defaults>(), __func__);
    material::default_color_map() = {};
    material::default_normal_map() = {};
    return true;
}

auto defaults::init_assets(rtti::context& ctx) -> bool
{
    auto& manager = ctx.get<asset_manager>();
    {
        const auto id = "engine:/embedded/cube";
        auto instance = std::make_shared<mesh>();
        instance->create_cube(gfx::mesh_vertex::get_layout(), 1.0f, 1.0f, 1.0f, 1, 1, 1, mesh_create_origin::center);
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/sphere";
        auto instance = std::make_shared<mesh>();
        instance->create_sphere(gfx::mesh_vertex::get_layout(), 0.5f, 20, 20, mesh_create_origin::center);
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/plane";
        auto instance = std::make_shared<mesh>();
        instance->create_plane(gfx::mesh_vertex::get_layout(), 10.0f, 10.0f, 1, 1, mesh_create_origin::center);
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/cylinder";
        auto instance = std::make_shared<mesh>();
        instance->create_cylinder(gfx::mesh_vertex::get_layout(), 0.5f, 2.0f, 20, 20, mesh_create_origin::center);
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/capsule";
        auto instance = std::make_shared<mesh>();
        instance->create_capsule(gfx::mesh_vertex::get_layout(), 0.5f, 2.0f, 20, 20, mesh_create_origin::center);
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/cone";
        auto instance = std::make_shared<mesh>();
        instance->create_cone(gfx::mesh_vertex::get_layout(), 0.5f, 0.0f, 2, 20, 20, mesh_create_origin::bottom);
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/torus";
        auto instance = std::make_shared<mesh>();
        instance->create_torus(gfx::mesh_vertex::get_layout(), 1.0f, 0.5f, 20, 20, mesh_create_origin::center);
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/teapot";
        auto instance = std::make_shared<mesh>();
        instance->create_teapot(gfx::mesh_vertex::get_layout());
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/icosahedron";
        auto instance = std::make_shared<mesh>();
        instance->create_icosahedron(gfx::mesh_vertex::get_layout());
        manager.get_asset_from_instance(id, instance);
    }
    {
        const auto id = "engine:/embedded/dodecahedron";
        auto instance = std::make_shared<mesh>();
        instance->create_dodecahedron(gfx::mesh_vertex::get_layout());
        manager.get_asset_from_instance(id, instance);
    }

    for(int i = 0; i < 20; ++i)
    {
        const auto id = std::string("engine:/embedded/icosphere") + std::to_string(i);
        auto instance = std::make_shared<mesh>();
        instance->create_icosphere(gfx::mesh_vertex::get_layout(), i);
        manager.get_asset_from_instance(id, instance);
    }

    {
        material::default_color_map() = manager.get_asset<gfx::texture>("engine:/data/textures/default_color.dds");
        material::default_normal_map() = manager.get_asset<gfx::texture>("engine:/data/textures/default_normal.dds");
    }
    {
        const auto id = "engine:/embedded/standard";
        auto instance = std::make_shared<pbr_material>();
        auto asset = manager.get_asset_from_instance<material>(id, instance);

        model::default_material() = asset;
    }

    {
        const auto id = "engine:/embedded/fallback";
        auto instance = std::make_shared<pbr_material>();
        instance->set_emissive_color(math::color::purple());
        instance->set_base_color(math::color::purple());
        instance->set_roughness(1.0f);
        auto asset = manager.get_asset_from_instance<material>(id, instance);

        model::fallback_material() = asset;
    }

    return true;
}

auto defaults::create_embedded_mesh_entity(rtti::context& ctx, scene& scn, const std::string& name) -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    const auto id = "engine:/embedded/" + string_utils::to_lower(name);

    auto lod = am.get_asset<mesh>(id);
    model model;
    model.set_lod(lod, 0);
    model.set_material(am.get_asset<material>("engine:/embedded/standard"), 0);
    auto object = scn.create_entity();
    object.get_or_emplace<tag_component>().tag = name;

    auto& transf_comp = object.get_or_emplace<transform_component>();

    auto bounds = lod.get()->get_bounds();
    transf_comp.set_position_local({0.0f, bounds.get_extents().y, 0.0f});

    auto& model_comp = object.get_or_emplace<model_component>();
    model_comp.set_casts_shadow(true);
    model_comp.set_casts_reflection(false);
    model_comp.set_model(model);

    return object;
}

auto defaults::create_prefab_at(rtti::context& ctx, scene& scn, const std::string& key, math::vec3 pos) -> entt::handle
{
    auto& am = ctx.get<asset_manager>();
    auto asset = am.get_asset<prefab>(key);

    auto object = scn.instantiate(asset);

    auto& trans_comp = object.get<transform_component>();
    trans_comp.set_position_global(pos);

    return object;
}

auto defaults::create_prefab_at(rtti::context& ctx,
                                scene& scn,
                                const std::string& key,
                                const camera& cam,
                                math::vec2 pos) -> entt::handle
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
    auto asset = am.get_asset<mesh>(key);

    model mdl;
    mdl.set_lod(asset, 0);

    std::string name = fs::path(key).stem().string();
    auto object = scn.create_entity(name);


    // Add component and configure it.
    auto& model_comp = object.emplace<model_component>();
    model_comp.set_casts_shadow(true);
    model_comp.set_casts_reflection(false);
    model_comp.set_model(mdl);

    auto& trans_comp = object.get<transform_component>();
    trans_comp.set_position_global(pos);

    model_comp.update_armature();

    return object;
}

auto defaults::create_mesh_entity_at(rtti::context& ctx,
                                     scene& scn,
                                     const std::string& key,
                                     const camera& cam,
                                     math::vec2 pos) -> entt::handle
{
    math::vec3 projected_pos{0.0f, 0.0f, 0.0f};
    cam.viewport_to_world(pos,
                          math::plane::from_point_normal(math::vec3{0.0f, 0.0f, 0.0f}, math::vec3{0.0f, 1.0f, 0.0f}),
                          projected_pos,
                          false);

    return create_mesh_entity_at(ctx, scn, key, projected_pos);
}

auto defaults::create_light_entity(rtti::context& ctx, scene& scn, light_type type, const std::string& name)
    -> entt::handle
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
    auto camera = create_camera_entity(ctx, scn, "Main Camera");
    camera.emplace<audio_listener_component>();

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

auto defaults::create_default_3d_scene_for_preview(rtti::context& ctx, scene& scn, const usize32_t& size)
    -> entt::handle
{
    auto camera = create_camera_entity(ctx, scn, "Main Camera");
    {
        auto& transf_comp = camera.get<transform_component>();
        transf_comp.set_position_local({5.0f, 5.0f, -10.0f});

        auto& camera_comp = camera.get<camera_component>();
        camera_comp.set_viewport_size(size);
    }

    {
        auto object = create_light_entity(ctx, scn, light_type::directional, "Sky & Directional");

        auto& light_comp = object.get_or_emplace<light_component>();
        auto light = light_comp.get_light();
        light.casts_shadows = false;
        light_comp.set_light(light);

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

    return camera;
}

template<>
void defaults::create_default_3d_scene_for_asset_preview(rtti::context& ctx,
                                                         scene& scn,
                                                         const asset_handle<material>& asset,
                                                         const usize32_t& size)
{
    auto camera = create_default_3d_scene_for_preview(ctx, scn, size);

    {
        auto object = create_embedded_mesh_entity(ctx, scn, "Sphere");
        auto& model_comp = object.get<model_component>();
        auto model = model_comp.get_model();
        model.set_material(asset, 0);
        model_comp.set_model(model);
        model_comp.set_casts_shadow(false);
        model_comp.set_casts_reflection(false);

        focus_camera_on_bounds(camera, calc_bounds_sphere_global(object));
    }
}

template<>
void defaults::create_default_3d_scene_for_asset_preview(rtti::context& ctx,
                                                         scene& scn,
                                                         const asset_handle<prefab>& asset,
                                                         const usize32_t& size)
{
    auto camera = create_default_3d_scene_for_preview(ctx, scn, size);

    {
        auto object = scn.instantiate(asset);

        if(auto model_comp = object.try_get<model_component>())
        {
            model_comp->set_casts_shadow(false);
            model_comp->set_casts_reflection(false);
        }

        focus_camera_on_bounds(camera, calc_bounds_sphere_global(object));
    }
}

template<>
void defaults::create_default_3d_scene_for_asset_preview(rtti::context& ctx,
                                                         scene& scn,
                                                         const asset_handle<mesh>& asset,
                                                         const usize32_t& size)
{
    auto camera = create_default_3d_scene_for_preview(ctx, scn, size);

    {
        auto object = create_mesh_entity_at(ctx, scn, asset.id());

        if(auto model_comp = object.try_get<model_component>())
        {
            model_comp->set_casts_shadow(false);
            model_comp->set_casts_reflection(false);
        }

        focus_camera_on_bounds(camera, calc_bounds_sphere_global(object));
    }
}

void defaults::focus_camera_on_entity(entt::handle camera, entt::handle entity)
{
    if(camera.all_of<transform_component, camera_component>())
    {
        auto bounds = calc_bounds_global(entity);
        focus_camera_on_bounds(camera, bounds);
    }
}

math::bbox defaults::calc_bounds_global(entt::handle entity)
{
    const math::vec3 one = {1.0f, 1.0f, 1.0f};
    math::bbox bounds = math::bbox(-one, one);
    auto& ent_trans_comp = entity.get<transform_component>();
    {
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
                    bounds = mesh->get_bounds();
                }
            }
        }
        const auto& world = ent_trans_comp.get_transform_global();
        bounds = math::bbox::mul(bounds, world);
    }
    return bounds;
};

math::bsphere defaults::calc_bounds_sphere_global(entt::handle entity)
{
    auto box = calc_bounds_global(entity);
    math::bsphere result;
    result.position = box.get_center();
    auto extends = box.get_extents();
    result.radius = std::max(extends.x, std::max(extends.y, extends.z));

    return result;
};
} // namespace ace
