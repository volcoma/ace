#include "debugdraw_rendering.h"
#include <editor/editing/editing_manager.h>
#include <editor/ecs/editor_ecs.h>

#include <graphics/debugdraw.h>
#include <graphics/render_pass.h>

#include <engine/assets/asset_manager.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/events.h>

namespace ace
{
void debugdraw_rendering::draw_grid(uint32_t pass_id, const camera& cam, float opacity)
{
    grid_program_->begin();

    float grid_height = 0.0f;
    math::vec4 u_params(grid_height, cam.get_near_clip(), cam.get_far_clip(), opacity);
    grid_program_->set_uniform("u_params", u_params);

    auto topology = gfx::clip_quad(1.0f);
    gfx::set_state(topology |
                   BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
                   BGFX_STATE_WRITE_Z |
                   BGFX_STATE_DEPTH_TEST_LEQUAL |
                   BGFX_STATE_BLEND_ALPHA);
    gfx::submit(pass_id, grid_program_->native_handle());
    gfx::set_state(BGFX_STATE_DEFAULT);


    grid_program_->end();

}

void debugdraw_rendering::draw_shapes(uint32_t pass_id, const camera& cam, entt::handle e)
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
            aabb.min.x = bounds.min.x;
            aabb.min.y = bounds.min.y;
            aabb.min.z = bounds.min.z;
            aabb.max.x = bounds.max.x;
            aabb.max.y = bounds.max.y;
            aabb.max.z = bounds.max.z;
            dd.encoder.pushTransform(&world_transform);
            dd.encoder.draw(aabb);
            dd.encoder.popTransform();
        }

    }

    if(e.all_of<light_component>())
    {
        const auto light_comp = e.get<light_component>();
        const auto& light = light_comp.get_light();
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
                dd.encoder.drawCone({to.x, to.y, to.z}, {from.x, from.y, from.z}, oposite);
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
                dd.encoder.drawCone({to.x, to.y, to.z}, {from.x, from.y, from.z}, oposite);
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
            math::vec3 to1 = from1 + transform_comp.get_z_axis_local() * 2.0f;
            dd.encoder.drawCylinder({from1.x, from1.y, from1.z}, {to1.x, to1.y, to1.z}, 0.1f);
            math::vec3 from2 = to1;
            math::vec3 to2 = from2 + transform_comp.get_z_axis_local() * 1.5f;
            dd.encoder.drawCone({from2.x, from2.y, from2.z}, {to2.x, to2.y, to2.z}, 0.5f);
        }
    }

    if(e.all_of<reflection_probe_component>())
    {
        const auto probe_comp = e.get<reflection_probe_component>();
        const auto& probe = probe_comp.get_probe();
        if(probe.type == probe_type::box)
        {
            DebugDrawEncoderScopePush scope(dd.encoder);
            dd.encoder.setColor(0xff00ff00);
            dd.encoder.setWireframe(true);
            dd.encoder.pushTransform(&world_transform);
            bx::Aabb aabb;
            aabb.min.x = -probe.box_data.extents.x;
            aabb.min.y = -probe.box_data.extents.y;
            aabb.min.z = -probe.box_data.extents.z;
            aabb.max.x = probe.box_data.extents.x;
            aabb.max.y = probe.box_data.extents.y;
            aabb.max.z = probe.box_data.extents.z;
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
        const auto model_comp = e.get<model_component>();
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
                dd.encoder.setColor(0xff00ff00);
                dd.encoder.setWireframe(true);
                dd.encoder.pushTransform(&world_transform);
                bx::Aabb aabb;
                aabb.min.x = bounds.min.x;
                aabb.min.y = bounds.min.y;
                aabb.min.z = bounds.min.z;
                aabb.max.x = bounds.max.x;
                aabb.max.y = bounds.max.y;
                aabb.max.z = bounds.max.z;
                dd.encoder.draw(aabb);
                dd.encoder.popTransform();
            }
        }
    }
}

void debugdraw_rendering::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& em = ctx.get<editing_manager>();
    auto& eecs = ctx.get<editor_ecs>();

    auto& editor_camera = eecs.editor_camera;
    auto& selected = em.selection_data.object;
	if(!editor_camera)
		return;

	auto& camera_comp = editor_camera.get<camera_component>();
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
        draw_shapes(pass.id, camera, e);
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
    auto& ev = ctx.get<events>();
    ev.on_frame_render.connect(sentinel_, 800, this, &debugdraw_rendering::on_frame_render);

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
} // namespace editor