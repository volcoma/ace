#include "pipeline.h"
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>

#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>

namespace ace
{
namespace rendering
{
auto pipeline::gather_visible_models(scene& scn, const math::frustum* frustum, visibility_flags query)
    -> visibility_set_models_t
{
    visibility_set_models_t result;

    scn.registry->view<transform_component, model_component>().each(
        [&](auto e, auto&& transform_comp, auto&& model_comp)
        {
            auto entity = scn.create_entity(e);

            if((query & visibility_query::is_static) && !model_comp.is_static())
            {
                return;
            }

            if((query & visibility_query::is_reflection_caster) && !model_comp.casts_reflection())
            {
                return;
            }

            if((query & visibility_query::is_shadow_caster) && !model_comp.casts_shadow())
            {
                return;
            }

            if(frustum)
            {
                const auto& world_transform = transform_comp.get_transform_global();
                const auto& world_bounds = model_comp.get_world_bounds();
                const auto& local_bounds = model_comp.get_local_bounds();

                // Test the bounding box of the mesh
                if(frustum->test_obb(local_bounds, world_transform))
                //if(frustum->test_aabb(world_bounds))
                {
                    // Only dirty mesh components.
                    if(query & visibility_query::is_dirty)
                    {
                        //                        if(transform_comp.is_touched() || model_comp.is_touched())
                        {
                            result.emplace_back(entity);
                        }
                    } // End if dirty_only
                    else
                    {
                        result.emplace_back(entity);
                    }

                } // Enf if visble
            }
            else
            {
                // Only dirty mesh components.
                if(query & visibility_query::is_dirty)
                {
                    //                    if(transform_comp.is_touched() || model_comp.is_touched())
                    {
                        result.emplace_back(entity);
                    }
                } // End if dirty_only
                else
                {
                    result.emplace_back(entity);
                }
            }
        });

    return result;
}

} // namespace rendering
} // namespace ace
