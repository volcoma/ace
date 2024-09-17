#include "animation_system.h"
#include <engine/animation/animation.h>
#include <engine/animation/ecs/components/animation_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/rendering/mesh.h>

#include <engine/ecs/ecs.h>

#include <logging/logging.h>

namespace ace
{

auto animation_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto animation_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void animation_system::on_frame_update(scene& scn, delta_t dt)
{
    scn.registry->view<transform_component, model_component, animation_component>().each(
        [&](auto e, auto&& transform_comp, auto&& model_comp, auto&& animation_comp)
        {
            auto& player = animation_comp.get_player();

            if(player.set_animation(animation_comp.get_animation()))
            {
                player.play();
            }

            player.update(dt,
                          [&](const std::string& node_id, size_t node_index, const math::transform& transform)
                          {
                              auto armature = model_comp.get_armature_by_index(node_index);
                              if(armature)
                              {
                                  auto& armature_transform_comp = armature.template get<transform_component>();
                                  armature_transform_comp.set_transform_local(transform);
                              }
                          });
        });
}

} // namespace ace
