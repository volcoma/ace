#include "rendering_path.h"
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>

#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>

namespace ace
{

auto rendering_path::gather_visible_models(scene& scn, const math::frustum* frustum, visibility_flags query)
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

            auto lod = model_comp.get_model().get_lod(0);

            // If mesh isnt loaded yet skip it.
            if(!lod.is_ready())
                return;

            const auto& mesh = lod.get();

            if(frustum)
            {

                const auto& world_transform = transform_comp.get_transform_global();

                const auto& bounds = mesh->get_bounds();

                // Test the bounding box of the mesh
                if(frustum->test_obb(bounds, world_transform))
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


auto rendering_path::render_scene(scene& scn, delta_t dt) -> std::shared_ptr<gfx::frame_buffer>
{
    std::shared_ptr<gfx::frame_buffer> output{};
    scn.registry->view<camera_component>().each(
        [&](auto e, auto&& camera_comp)
        {
            auto& camera = camera_comp.get_camera();
            auto& render_view = camera_comp.get_render_view();
            auto& storage = camera_comp.get_storage();

            output = camera_render_full(scn, camera, storage, render_view, dt);
        });

    return output;
}

void rendering_path::render_scene(const std::shared_ptr<gfx::frame_buffer>& output, scene& scn, delta_t dt)
{
    scn.registry->view<camera_component>().each(
        [&](auto e, auto&& camera_comp)
        {
            auto& camera = camera_comp.get_camera();
            auto& render_view = camera_comp.get_render_view();
            auto& storage = camera_comp.get_storage();

            camera_render_full(output, scn, camera, storage, render_view, dt);
        });
}

} // namespace ace
