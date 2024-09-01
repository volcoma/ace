#include "animation_system.h"
#include <engine/animation/animation.h>
#include <engine/ecs/components/animation_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/rendering/mesh.h>

#include <engine/ecs/ecs.h>

#include <logging/logging.h>

namespace ace
{

void blend_bone_transforms(const pose_transform* pose0,
                           const pose_transform* pose1,
                           const float w,
                           pose_transform* result)
{
    const auto& pose0_bone_transforms = pose0->transforms;
    const auto& pose1_bone_transforms = pose1->transforms;
    auto& result_bone_transforms = result->transforms;
    auto pose0_bones = pose0_bone_transforms.size();
    auto pose1_bones = pose1_bone_transforms.size();
    auto result_bones = result_bone_transforms.size();

    // Note (0x): lerp for root bone translation is not ideal (e.g. speed will not be preserved when combining forwards
    // movement and rightwards movement to make a diagonal)
    result_bone_transforms[0].set_translation(
        math::mix(pose0_bone_transforms[0].get_translation(), pose1_bone_transforms[0].get_translation(), w));
    result_bone_transforms[0].set_rotation(
        math::slerp(pose0_bone_transforms[0].get_rotation(), pose1_bone_transforms[0].get_rotation(), w));
    result_bone_transforms[0].set_scale(
        math::mix(pose0_bone_transforms[0].get_scale(), pose1_bone_transforms[0].get_scale(), w));

    size_t bones_count = std::min({pose0_bones, pose1_bones, result_bones});
    for(size_t i = 1; i < bones_count; ++i)
    {
        result_bone_transforms[i].set_translation(
            math::mix(pose0_bone_transforms[i].get_translation(), pose1_bone_transforms[i].get_translation(), w));
        result_bone_transforms[i].set_rotation(
            math::slerp(pose0_bone_transforms[i].get_rotation(), pose1_bone_transforms[i].get_rotation(), w));
        result_bone_transforms[i].set_scale(
            math::mix(pose0_bone_transforms[i].get_scale(), pose1_bone_transforms[i].get_scale(), w));
    }
}

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
            const auto& model = model_comp.get_model();

            auto& player = animation_comp.player;

            if(animation_comp.animation)
            {
                if(player.set_animation(animation_comp.animation))
                {
                    player.play();
                }

                player.update(dt,
                              [&](const std::string& node_id, size_t node_index, const math::transform& transform)
                              {
                                  auto armature = model_comp.get_armature_by_index(node_index);

                                  if(armature)
                                  {
                                      armature.template get<transform_component>().set_transform_local(transform);
                                  }
                              });
            }
        });
}

} // namespace ace
