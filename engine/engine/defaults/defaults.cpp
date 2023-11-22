#include "defaults.h"

#include <logging/logging.h>
#include <engine/assets/asset_manager.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>

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
        const auto id = "embedded:/sphere";
        auto instance = std::make_shared<mesh>();
        instance->create_sphere(gfx::mesh_vertex::get_layout(), 0.5f, 20, 20, mesh_create_origin::center);
        manager.load_asset_from_instance(id, instance);
    }
    {
        const auto id = "embedded:/cube";
        auto instance = std::make_shared<mesh>();
        instance->create_cube(gfx::mesh_vertex::get_layout(), 1.0f, 1.0f, 1.0f, 1, 1, 1, mesh_create_origin::center);
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
        manager.load_asset_from_instance<material>(id, instance);
    }

    {
        const auto id = "embedded:/fallback";
        auto instance = std::make_shared<standard_material>();
        instance->set_emissive_color(math::color::purple());
        instance->set_roughness(1.0f);
        manager.load_asset_from_instance<material>(id, instance);
    }

    return true;
}

} // namespace ace
