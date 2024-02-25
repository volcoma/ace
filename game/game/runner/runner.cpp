#include "runner.h"
#include <engine/assets/asset_manager.h>
#include <engine/ecs/systems/rendering_path.h>
#include <engine/events.h>
#include <engine/rendering/renderer.h>
#include <graphics/debugdraw.h>

#include <logging/logging.h>

namespace ace
{

auto runner::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();

    ev.on_frame_update.connect(sentinel_, this, &runner::on_frame_update);
    ev.on_frame_render.connect(sentinel_, this, &runner::on_frame_render);

    fs::path project_path = "C:/";
    project_path = project_path / "Workspace" / "github" / "ace" / "projects";

    fs::add_path_protocol("app", project_path);

    auto& am = ctx.get<asset_manager>();
    auto scn = am.load<scene_prefab>("app:/data/Startup.spfb");
    auto& ec = ctx.get<ecs>();
    return ec.get_scene().load_from(scn);
}

auto runner::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void runner::on_frame_update(rtti::context& ctx, delta_t dt)
{
}

void runner::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& rend = ctx.get<renderer>();

    auto& path = ctx.get<rendering_path>();
    auto& ec = ctx.get<ecs>();
    auto& scene = ec.get_scene();

    auto render_target = path.render_scene(scene, dt);

    auto& window = rend.get_main_window();

    gfx::dd_raii dd(window->get_present_pass().id);


    auto radius = 1.0f;;
    DebugDrawEncoderScopePush scope(dd.encoder);
    dd.encoder.setColor(0xff00ff00);
    dd.encoder.setWireframe(true);
    math::vec3 center(0.0f, 0.0f, 0.0f);
    dd.encoder.drawCircle(Axis::X, center.x, center.y, center.z, radius);
    dd.encoder.drawCircle(Axis::Y, center.x, center.y, center.z, radius);
    dd.encoder.drawCircle(Axis::Z, center.x, center.y, center.z, radius);

    // gfx::render_pass pass("game_fill_window_surface");
    // gfx::blit(pass.id, window->get_surface()->native_handle(), 0, 0, render_target->get_texture()->native_handle());
}
} // namespace ace
