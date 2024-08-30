#include "bone_system.h"
#include <engine/rendering/mesh.h>

#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>

#include <logging/logging.h>

namespace ace
{

auto bone_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto bone_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void bone_system::on_frame_update(scene& scn, delta_t dt)
{
    scn.registry->view<transform_component, model_component>().each(
        [&](auto e, auto&& transform_comp, auto&& model_comp)
        {
            const auto& model = model_comp.get_model();

            model_comp.update_armature();
        });
}

} // namespace ace
