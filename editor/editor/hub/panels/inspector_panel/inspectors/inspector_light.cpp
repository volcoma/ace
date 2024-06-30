#include "inspector_light.h"
#include "inspectors.h"

namespace ace
{

auto inspector_light_component::inspect(rtti::context& ctx,
                                        rttr::variant& var,
                                        const var_info& info,
                                        const meta_getter& get_metadata) -> inspect_result
{
    auto& data = *var.get_value<light_component*>();
    auto light_val = data.get_light();
    auto result = ::ace::inspect(ctx, light_val);

    if(light_val.type == light_type::spot)
    {
        result |= ::ace::inspect(ctx, light_val.spot_data);
    }
    else if(light_val.type == light_type::point)
    {
        result |= ::ace::inspect(ctx, light_val.point_data);
    }
    else if(light_val.type == light_type::directional)
    {
        result |= ::ace::inspect(ctx, light_val.directional_data);
    }

    if(data.get_light().casts_shadows)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
        if(ImGui::TreeNode("Shadow"))
        {
            ImGui::TreePush("Shadow");
            result |= ::ace::inspect(ctx, light_val.shadow_params);

            ImGui::AlignTextToFramePadding();
            ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
            if(ImGui::TreeNode("Params"))
            {
                ImGui::TreePush("Specific");

                if(light_val.type == light_type::spot)
                {
                    result |= ::ace::inspect(ctx, light_val.spot_data.shadow_params);
                }
                else if(light_val.type == light_type::point)
                {
                    result |= ::ace::inspect(ctx, light_val.point_data.shadow_params);
                }
                else if(light_val.type == light_type::directional)
                {
                    result |= ::ace::inspect(ctx, light_val.directional_data.shadow_params);
                }

                ImGui::TreePop();
                ImGui::TreePop();
            }

            ImGui::AlignTextToFramePadding();
            if(ImGui::TreeNode("Maps"))
            {
                ImGui::TreePush("Maps");

                auto& generator = data.get_shadowmap_generator();

                auto depth_type = generator.get_depth_type();

                ImGui::BeginGroup();
                ImGui::Image(
                    ImGui::ToTex(generator.get_rt_texture(0), 0, generator.get_depth_render_program(depth_type)).id,
                    ImVec2(256, 250));

                if(light_val.type == light_type::directional)
                {
                    for(uint8_t ii = 1; ii < light_val.directional_data.shadow_params.num_splits; ++ii)
                    {
                        ImGui::Image(ImGui::ToTex(generator.get_rt_texture(ii),
                                                  0,
                                                  generator.get_depth_render_program(depth_type))
                                         .id,
                                     ImVec2(256, 256));
                    }
                }

                ImGui::EndGroup();

                ImGui::TreePop();
                ImGui::TreePop();
            }

            ImGui::TreePop();
            ImGui::TreePop();
        }
    }

    if(result.changed)
    {
        data.set_light(light_val);
        return result;
    }

    return result;
}

auto inspector_reflection_probe_component::inspect(rtti::context& ctx,
                                                   rttr::variant& var,
                                                   const var_info& info,
                                                   const meta_getter& get_metadata) -> inspect_result
{
    auto& data = *var.get_value<reflection_probe_component*>();
    auto probe = data.get_probe();

    auto result = ::ace::inspect(ctx, probe);

    if(probe.type == probe_type::box)
    {
        result |= ::ace::inspect(ctx, probe.box_data);
    }
    else if(probe.type == probe_type::sphere)
    {
        result |= ::ace::inspect(ctx, probe.sphere_data);
    }

    if(result.changed)
    {
        data.set_probe(probe);
        return result;
    }

    return result;
}
} // namespace ace
