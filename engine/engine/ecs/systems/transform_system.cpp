#include "transform_system.h"

#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/profiler/profiler.h>

#include <logging/logging.h>

#define POOLSTL_STD_SUPPLEMENT 1
#include <poolstl/poolstl.hpp>

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
    APP_SCOPE_PERF("Transform System");

    // Create a view for entities with transform_component and submesh_component
    auto view_root = scn.registry->view<transform_component, root_component>();

    // Use std::for_each with the view's iterators
    std::for_each(std::execution::par,
                  view_root.begin(),
                  view_root.end(),
                  [&view_root](entt::entity entity)
                  {
                      auto& transform_comp = view_root.get<transform_component>(entity);

                      transform_comp.resolve_transform_global();
                  });
}

} // namespace ace
