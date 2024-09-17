#include "transform_system.h"

#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>

#include <logging/logging.h>

namespace ace
{

auto transform_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto transform_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void transform_system::on_frame_update(scene& scn, delta_t dt)
{
    scn.registry->view<transform_component, root_component>().each(
        [&](auto e, auto&& transform, auto&& root)
        {
            //transform.resolve_transform_global();
            transform.get_transform_global();
        });

}


} // namespace ace
