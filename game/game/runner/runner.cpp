#include "runner.h"
#include <engine/assets/asset_manager.h>
#include <engine/ecs/systems/rendering_path.h>
#include <engine/ecs/components/camera_component.h>

#include <engine/events.h>
#include <engine/rendering/renderer.h>


#include <logging/logging.h>

namespace ace
{

auto runner::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();

    ev.on_frame_update.connect(sentinel_, this, &runner::on_frame_update);
    ev.on_frame_render.connect(sentinel_, this, &runner::on_frame_render);

    auto& am = ctx.get<asset_manager>();

    auto scn = am.load<scene_prefab>("app:/data/Startup.spfb");
    if(!scn)
    {
        APPLOG_CRITICAL("Failed to load initial scene {}", scn.id());
        return false;
    }
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
    auto& window = rend.get_main_window();
    auto size = window->get_window().get_size();

    scene.registry->view<camera_component>().each(
        [&](auto e, auto&& camera_comp)
        {
            camera_comp.set_viewport_size({size.w, size.h});
        });

    path.render_scene(window->get_surface(), scene, dt);
}
} // namespace ace
