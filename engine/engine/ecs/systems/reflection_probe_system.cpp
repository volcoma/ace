#include "reflection_probe_system.h"
#include <engine/events.h>

#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>

#include <logging/logging.h>

namespace ace
{

auto reflection_probe_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto reflection_probe_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void reflection_probe_system::on_frame_update(scene& scn, delta_t dt)
{
    scn.registry->view<transform_component, reflection_probe_component>().each(
        [&](auto e, auto&& transform, auto&& probe)
        {
            probe.update();
        });
}

} // namespace ace
