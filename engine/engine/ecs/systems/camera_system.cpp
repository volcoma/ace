#include "camera_system.h"

#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>

#include <logging/logging.h>

namespace ace
{

auto camera_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto camera_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void camera_system::on_frame_update(scene& scn, delta_t dt)
{
    scn.registry->view<transform_component, camera_component>().each(
        [&](auto e, auto&& transform, auto&& camera)
        {
            camera.update(transform.get_transform_global());
        });
}


} // namespace ace
