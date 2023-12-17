#include "camera_system.h"
#include <engine/events.h>

#include "../components/camera_component.h"
#include "../components/transform_component.h"

namespace ace
{
void camera_system::on_frame_update(rtti::context& ctx, delta_t /*dt*/)
{
    // auto& ec = ctx.get<ecs>();

    // ec.registry.view<transform_component, camera_component>().each(
    //             [&](auto e, auto&& transform, auto&& camera)
    //             {
    //                 camera.update(transform.get_transform_global());
    //             });
}

auto camera_system::init(rtti::context& ctx) -> bool
{
    auto& ev = ctx.get<events>();
    ev.on_frame_update.connect(sentinel_, this, &camera_system::on_frame_update);
    return true;

}

auto camera_system::deinit(rtti::context& ctx) -> bool
{
    return true;
}

} // namespace runtime
