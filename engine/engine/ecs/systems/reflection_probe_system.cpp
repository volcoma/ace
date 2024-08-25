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
    // Sort based on reflection_probe_component's method and max range
    scn.registry->sort<reflection_probe_component>(
        [&](const auto& lhs, const auto& rhs)
        {
            const auto& lhs_probe = lhs.get_probe();
            const auto& rhs_probe = rhs.get_probe();

            // Environment probes should be last
            if(lhs_probe.method != rhs_probe.method)
            {
                return lhs_probe.method < rhs_probe.method;
            }

            // If the reflection methods are the same, compare based on the maximum range
            return lhs_probe.get_max_range() > rhs_probe.get_max_range();
        });
    scn.registry->view<transform_component, reflection_probe_component>().each(
        [&](auto e, auto&& transform, auto&& probe)
        {
            probe.update();
        });
}

} // namespace ace
