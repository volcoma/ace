#include "gizmos_renderer.h"
#include <editor/editing/editing_manager.h>
#include <editor/events.h>

#include <graphics/render_pass.h>

#include <engine/events.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>

#include <engine/physics/backend/bullet/bullet_backend.h>

#include "gizmos/gizmos.h"

namespace ace
{

void gizmos_renderer::draw_grid(uint32_t pass_id, const camera& cam, float opacity)
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

void gizmos_renderer::on_frame_render(rtti::context& ctx, entt::handle camera_entity)
{
    if(!camera_entity)
        return;

    const auto& ec = ctx.get<ecs>();

    const auto& scene = ec.get_scene();
    auto& em = ctx.get<editing_manager>();

    auto& selected = em.selection_data.object;

    auto& am = ctx.get<asset_manager>();
    auto& camera_comp = camera_entity.get<camera_component>();
    const auto& rview = camera_comp.get_render_view();
    const auto& camera = camera_comp.get_camera();
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();
    const auto& obuffer = rview.fbo_get("OBUFFER");

    gfx::render_pass pass("debug_draw_pass");
    pass.bind(obuffer.get());
    pass.set_view_proj(view, proj);

    gfx::dd_raii dd(pass.id);

    bullet_backend::draw_system_gizmos(ctx, camera, dd);

    draw_gizmo_var(ctx, selected, camera, dd);

    if(em.show_grid)
    {
        draw_grid(pass.id, camera, em.grid_data.opacity);
    }
}

gizmos_renderer::gizmos_renderer()
{
}

gizmos_renderer::~gizmos_renderer()
{
}

bool gizmos_renderer::init(rtti::context& ctx)
{
    auto& am = ctx.get<asset_manager>();

    {
        auto vs = am.get_asset<gfx::shader>("editor:/data/shaders/vs_wf_wireframe.sc");
        auto fs = am.get_asset<gfx::shader>("editor:/data/shaders/fs_wf_wireframe.sc");
        wireframe_program_ = std::make_unique<gpu_program>(vs, fs);
    }

    {
        auto vs = am.get_asset<gfx::shader>("editor:/data/shaders/vs_grid.sc");
        auto fs = am.get_asset<gfx::shader>("editor:/data/shaders/fs_grid.sc");
        grid_program_ = std::make_unique<gpu_program>(vs, fs);
    }

    return true;
}

bool gizmos_renderer::deinit(rtti::context& ctx)
{
    wireframe_program_.reset();
    grid_program_.reset();
    return true;
}
} // namespace ace
