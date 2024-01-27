#include "bone_system.h"
#include <engine/events.h>
#include <engine/rendering/mesh.h>

#include <engine/ecs/ecs.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/components/model_component.h>

#include <logging/logging.h>

namespace ace
{
namespace
{

auto get_transforms_for_bones(const std::vector<entt::handle>& bone_entities) -> std::vector<math::transform>
{
    std::vector<math::transform> result;
    if(!bone_entities.empty())
    {
        result.reserve(bone_entities.size());
        for(const auto& e : bone_entities)
        {
            if(e)
            {
                const auto bone_transform = e.try_get<transform_component>();
                if(bone_transform)
                {
                    result.emplace_back(bone_transform->get_transform_global());
                    continue;
                }
            }

            result.emplace_back();
        }
    }

    return result;
}
}

auto bone_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();
    ev.on_frame_update.connect(sentinel_, this, &bone_system::on_frame_update);

    return true;
}

auto bone_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void bone_system::on_frame_update(rtti::context &ctx, delta_t dt)
{
    auto& ec = ctx.get<ecs>();

    ec.get_scene().registry.view<transform_component, model_component>().each(
        [&](auto e, auto&& transform_comp, auto&& model_comp)
        {
            auto entity = ec.get_scene().create_entity(e);

            const auto& model = model_comp.get_model();
            auto lod = model.get_lod(0);


            // If mesh isnt loaded yet skip it.
            if(!lod)
                return;

            const auto& mesh = lod.get();

            const auto& skin_data = mesh.get_skin_bind_data();

                   // Has skinning data?
            if(skin_data.has_bones())
            {
                const auto& bone_entities = model_comp.get_bone_entities();
                auto transforms = get_transforms_for_bones(bone_entities);
                model_comp.set_bone_transforms(std::move(transforms));
            }
        });

}

}
