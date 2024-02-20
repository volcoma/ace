#include "gizmos.h"
#include "edyn/comp/orientation.hpp"
#include "engine/ecs/components/physics/rigidbody_ex.h"
#include "physics/debugdraw.h"
#include <editor/editing/editing_manager.h>

#include <graphics/debugdraw.h>
#include <graphics/render_pass.h>

#include <engine/assets/asset_manager.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/physics_component.h>
#include <engine/ecs/components/transform_component.h>

#include <engine/events.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <variant>

namespace ace
{
namespace
{
auto to_bx(const math::vec3& data) -> bx::Vec3
{
    return {data.x, data.y, data.z};
}
} // namespace

void debugdraw_rendering::draw_grid(uint32_t pass_id, const camera& cam, float opacity)
{
    grid_program_->begin();

    float grid_height = 0.0f;
    math::vec4 u_params(grid_height, cam.get_near_clip(), cam.get_far_clip(), opacity);
    grid_program_->set_uniform("u_params", u_params);

    auto topology = gfx::clip_quad(1.0f);
    gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z |
                   BGFX_STATE_DEPTH_TEST_LEQUAL | BGFX_STATE_BLEND_ALPHA);
    gfx::submit(pass_id, grid_program_->native_handle());
    gfx::set_state(BGFX_STATE_DEFAULT);

    grid_program_->end();
}

void debugdraw_rendering::draw_shapes(asset_manager& am, uint32_t pass_id, const camera& cam, entt::handle e)
{
    if(!e || !e.all_of<transform_component>())
        return;

    auto& transform_comp = e.get<transform_component>();
    const auto& world_transform = transform_comp.get_transform_global();

    gfx::dd_raii dd(pass_id);

    if(e.all_of<camera_component>())
    {
        auto& selected_camera_comp = e.get<camera_component>();
        auto& selected_camera = selected_camera_comp.get_camera();
        const auto view_proj = selected_camera.get_view_projection();
        const auto bounds = selected_camera.get_local_bounding_box();
        DebugDrawEncoderScopePush scope(dd.encoder);
        dd.encoder.setColor(0xffffffff);
        dd.encoder.setWireframe(true);
        if(selected_camera.get_projection_mode() == projection_mode::perspective)
        {
            dd.encoder.drawFrustum(&view_proj);
        }
        else
        {
            bx::Aabb aabb;
            aabb.min = to_bx(bounds.min);
            aabb.max = to_bx(bounds.max);
            dd.encoder.pushTransform(&world_transform);
            dd.encoder.draw(aabb);
            dd.encoder.popTransform();
        }
    }

    if(e.all_of<light_component>())
    {
        const auto& light_comp = e.get<light_component>();
        const auto& light = light_comp.get_light();

        // auto tex = am.load<gfx::texture>("editor:/data/icons/material.png");
        // dd.encoder.setState(false, false, false);
        // dd.encoder.drawQuad(tex.get().native_handle(), to_bx(cam.z_unit_axis()),
        // to_bx(transform_comp.get_position_global()), 0.5f); dd.encoder.setState(true, true, false);

        if(light.type == light_type::spot)
        {
            auto adjacent = light.spot_data.get_range();
            {
                auto tan_angle = math::tan(math::radians(light.spot_data.get_outer_angle() * 0.5f));
                // oposite = tan * adjacent
                auto oposite = tan_angle * adjacent;
                DebugDrawEncoderScopePush scope(dd.encoder);
                dd.encoder.setColor(0xff00ff00);
                dd.encoder.setWireframe(true);
                dd.encoder.setLod(3);
                math::vec3 from = transform_comp.get_position_global();
                math::vec3 to = from + transform_comp.get_z_axis_local() * adjacent;
                dd.encoder.drawCone(to_bx(to), to_bx(from), oposite);
            }
            {
                auto tan_angle = math::tan(math::radians(light.spot_data.get_inner_angle() * 0.5f));
                // oposite = tan * adjacent
                auto oposite = tan_angle * adjacent;
                DebugDrawEncoderScopePush scope(dd.encoder);
                dd.encoder.setColor(0xff00ffff);
                dd.encoder.setWireframe(true);
                dd.encoder.setLod(3);
                math::vec3 from = transform_comp.get_position_global();
                math::vec3 to = from + transform_comp.get_z_axis_local() * adjacent;
                dd.encoder.drawCone(to_bx(to), to_bx(from), oposite);
            }
        }
        else if(light.type == light_type::point)
        {
            auto radius = light.point_data.range;
            DebugDrawEncoderScopePush scope(dd.encoder);
            dd.encoder.setColor(0xff00ff00);
            dd.encoder.setWireframe(true);
            math::vec3 center = transform_comp.get_position_global();
            dd.encoder.drawCircle(Axis::X, center.x, center.y, center.z, radius);
            dd.encoder.drawCircle(Axis::Y, center.x, center.y, center.z, radius);
            dd.encoder.drawCircle(Axis::Z, center.x, center.y, center.z, radius);
        }
        else if(light.type == light_type::directional)
        {
            DebugDrawEncoderScopePush scope(dd.encoder);
            dd.encoder.setLod(255);
            dd.encoder.setColor(0xff00ff00);
            dd.encoder.setWireframe(true);
            math::vec3 from1 = transform_comp.get_position_global();
            math::vec3 to1 = from1 + transform_comp.get_z_axis_local() * 1.0f;

            bx::Cylinder cylinder = {to_bx(from1), to_bx(to1), 0.1f};

            dd.encoder.draw(cylinder);
            math::vec3 from2 = to1;
            math::vec3 to2 = from2 + transform_comp.get_z_axis_local() * 0.5f;

            bx::Cone cone = {to_bx(from2), to_bx(to2), 0.25f};
            dd.encoder.draw(cone);
        }
    }

    if(e.all_of<reflection_probe_component>())
    {
        const auto& probe_comp = e.get<reflection_probe_component>();
        const auto& probe = probe_comp.get_probe();
        if(probe.type == probe_type::box)
        {
            DebugDrawEncoderScopePush scope(dd.encoder);
            dd.encoder.setColor(0xff00ff00);
            dd.encoder.setWireframe(true);
            dd.encoder.pushTransform(&world_transform);
            bx::Aabb aabb;
            aabb.min = to_bx(-probe.box_data.extents);
            aabb.max = to_bx(probe.box_data.extents);

            dd.encoder.draw(aabb);
            dd.encoder.popTransform();
        }
        else
        {
            auto radius = probe.sphere_data.range;
            DebugDrawEncoderScopePush scope(dd.encoder);
            dd.encoder.setColor(0xff00ff00);
            dd.encoder.setWireframe(true);
            math::vec3 center = transform_comp.get_position_global();
            dd.encoder.drawCircle(Axis::X, center.x, center.y, center.z, radius);
            dd.encoder.drawCircle(Axis::Y, center.x, center.y, center.z, radius);
            dd.encoder.drawCircle(Axis::Z, center.x, center.y, center.z, radius);
        }
    }

    if(e.all_of<model_component>())
    {
        const auto& model_comp = e.get<model_component>();
        const auto& model = model_comp.get_model();
        if(!model.is_valid())
            return;

        const auto lod = model.get_lod(0);
        if(!lod)
            return;
        const auto& mesh = lod.get();
        const auto& frustum = cam.get_frustum();
        const auto& bounds = mesh.get_bounds();
        // Test the bounding box of the mesh
        if(math::frustum::test_obb(frustum, bounds, world_transform))
        {
            // if(es->wireframe_selection)
            //{
            //	const float u_params[8] =
            //	{
            //		1.0f, 1.0f, 0.0f, 0.7f, //r,g,b,a
            //		1.0f, 0.0f, 0.0f, 0.0f  //thickness, unused, unused, unused
            //	};
            //	if (!_program)
            //		return;
            //
            //	model.render(
            //		pass.id,
            //		world_transform,
            //		false,
            //		false,
            //		false,
            //		BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA),
            //		0,
            //		_program.get(), [&u_params](program& p)
            //	{
            //		p.set_uniform("u_params", &u_params, 2);
            //	});
            //}
            // else
            {
                DebugDrawEncoderScopePush scope(dd.encoder);
                dd.encoder.setColor(0xffffffff);
                dd.encoder.setWireframe(true);
                dd.encoder.pushTransform(&world_transform);
                bx::Aabb aabb;
                aabb.min = to_bx(bounds.min);
                aabb.max = to_bx(bounds.max);
                dd.encoder.draw(aabb);
                dd.encoder.popTransform();
            }
        }
    }

    if(e.all_of<phyisics_component>())
    {
        edyn::scalar m_rigid_body_axes_size{0.15f};
        const auto& comp = e.get<phyisics_component>();

        const auto& def = comp.get_def();
        auto physics_entity = comp.get_simulation_entity();

        const auto& world_pos = world_transform.get_position();
        edyn::position pos;
        pos.x = world_pos.x;
        pos.y = world_pos.y;
        pos.z = world_pos.z;

        const auto& world_rot = world_transform.get_rotation();
        edyn::orientation orn;
        orn.x = world_rot.x;
        orn.y = world_rot.y;
        orn.z = world_rot.z;
        orn.w = world_rot.w;

        DebugDrawEncoderScopePush scope(dd.encoder);

        uint32_t color = 0xff00ff00;

        if(physics_entity)
        {
            if(physics_entity.any_of<edyn::sleeping_tag>())
            {
                color = 0x80000000;
            }
            else if(auto* resident = physics_entity.try_get<edyn::island_resident>();
                    resident && resident->island_entity != entt::null)
            {
                // color = m_registry->get<ColorComponent>(resident->island_entity);
            }
        }

        dd.encoder.setColor(color);
        dd.encoder.setWireframe(true);

        float trans[16];
        edyn::vector3 origin;

        if(physics_entity && physics_entity.all_of<edyn::center_of_mass>())
        {
            const auto& com = physics_entity.get<edyn::center_of_mass>();
            origin = edyn::to_world_space(-com, pos, orn);
        }
        else
        {
            origin = pos;
        }

        auto bxquat = to_bx(orn);
        float rot[16];
        bx::mtxFromQuaternion(rot, bxquat);

        float rotT[16];
        bx::mtxTranspose(rotT, rot);
        bx::mtxTranslate(trans, origin.x, origin.y, origin.z);

        float mtx[16];
        bx::mtxMul(mtx, rotT, trans);

        dd.encoder.pushTransform(mtx);

        if(def.shape)
        {
            std::visit(
                [&](auto&& s)
                {
                    draw(dd.encoder, s);
                },
                *def.shape);
        }

        dd.encoder.drawAxis(0, 0, 0, m_rigid_body_axes_size);

        dd.encoder.popTransform();
    }
}

void debugdraw_rendering::on_frame_render(rtti::context& ctx, entt::handle camera_entity)
{
    if(!camera_entity)
        return;

    auto& em = ctx.get<editing_manager>();

    auto& selected = em.selection_data.object;

    auto& am = ctx.get<asset_manager>();
    auto& camera_comp = camera_entity.get<camera_component>();
    auto& render_view = camera_comp.get_render_view();
    auto& camera = camera_comp.get_camera();
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();
    const auto& viewport_size = camera.get_viewport_size();
    const auto surface = render_view.get_output_fbo(viewport_size);
    const auto camera_posiiton = camera.get_position();

    gfx::render_pass pass("debug_draw_pass");
    pass.bind(surface.get());
    pass.set_view_proj(view, proj);

    if(selected && selected.is_type<entt::handle>())
    {
        auto e = selected.get_value<entt::handle>();
        draw_shapes(am, pass.id, camera, e);
    }

    if(em.show_grid)
    {
        draw_grid(pass.id, camera, em.grid_data.opacity);
    }
}

debugdraw_rendering::debugdraw_rendering()
{
}

debugdraw_rendering::~debugdraw_rendering()
{
}

bool debugdraw_rendering::init(rtti::context& ctx)
{
    auto& am = ctx.get<asset_manager>();

    {
        auto vs = am.load<gfx::shader>("editor:/data/shaders/vs_wf_wireframe.sc");
        auto fs = am.load<gfx::shader>("editor:/data/shaders/fs_wf_wireframe.sc");
        wireframe_program_ = std::make_unique<gpu_program>(vs, fs);
    }

    {
        auto vs = am.load<gfx::shader>("editor:/data/shaders/vs_grid.sc");
        auto fs = am.load<gfx::shader>("editor:/data/shaders/fs_grid.sc");
        grid_program_ = std::make_unique<gpu_program>(vs, fs);
    }

    return true;
}

bool debugdraw_rendering::deinit(rtti::context& ctx)
{
    wireframe_program_.reset();
    grid_program_.reset();
    return true;
}
} // namespace ace
