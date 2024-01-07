#include "inspector_math.h"
#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>

namespace ace
{
namespace
{
float DRAG_SPEED = 0.01f;

auto quat_to_vec4(math::quat q) -> math::vec4
{
    math::vec4 v;
    v.x = q.x;
    v.y = q.y;
    v.z = q.z;
    v.w = q.w;
    return v;
}
auto vec4_to_quat(math::vec4 v) -> math::quat
{
    math::quat q;
    q.x = v.x;
    q.y = v.y;
    q.z = v.z;
    q.w = v.w;
    return q;
}

bool DragFloat2(math::vec2& data, const var_info& info, std::array<const char*, 2> formats = {{"X:%.2f", "Y:%.2f"}})
{
    return ImGui::DragMultiFormatScalarN("##",
                                         ImGuiDataType_Float,
                                         math::value_ptr(data),
                                         2,
                                         DRAG_SPEED,
                                         nullptr,
                                         nullptr,
                                         formats.data());
}

bool DragFloat3(math::vec3& data,
                const var_info& info,
                std::array<const char*, 3> formats = {{"X:%.3f", "Y:%.3f", "Z:%.3f"}})
{
    return ImGui::DragMultiFormatScalarN("##",
                                         ImGuiDataType_Float,
                                         math::value_ptr(data),
                                         3,
                                         DRAG_SPEED,
                                         nullptr,
                                         nullptr,
                                         formats.data());
}

bool DragFloat4(math::vec4& data,
                const var_info& info,
                std::array<const char*, 4> formats = {{"X:%.3f", "Y:%.3f", "Z:%.3f", "W:%.3f"}})
{
    return ImGui::DragMultiFormatScalarN("##",
                                         ImGuiDataType_Float,
                                         math::value_ptr(data),
                                         4,
                                         DRAG_SPEED,
                                         nullptr,
                                         nullptr,
                                         formats.data());
}

bool DragVec2(math::vec2& data, const var_info& info, const char* format = "%.3f")
{
    return ImGui::DragVecN("##",
                           ImGuiDataType_Float,
                           math::value_ptr(data),
                           data.length(),
                           DRAG_SPEED,
                           nullptr,
                           nullptr,
                           format);
}

bool DragVec3(math::vec3& data, const var_info& info, const char* format = "%.3f")
{
    return ImGui::DragVecN("##",
                           ImGuiDataType_Float,
                           math::value_ptr(data),
                           data.length(),
                           DRAG_SPEED,
                           nullptr,
                           nullptr,
                           format);
}

bool DragVec4(math::vec4& data, const var_info& info, const char* format = "%.3f")
{
    return ImGui::DragVecN("##",
                           ImGuiDataType_Float,
                           math::value_ptr(data),
                           data.length(),
                           DRAG_SPEED,
                           nullptr,
                           nullptr,
                           format);
}

} // namespace

bool inspector_vec2::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata)
{
    auto data = var.get_value<math::vec2>();

    if(DragVec2(data, info))
    {
        var = data;
        return true;
    }

    return false;
}

bool inspector_vec3::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata)
{
    auto data = var.get_value<math::vec3>();

    if(DragVec3(data, info))
    {
        var = data;
        return true;
    }

    return false;
}

bool inspector_vec4::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata)
{
    auto data = var.get_value<math::vec4>();

    if(DragVec4(data, info))
    {
        var = data;
        return true;
    }

    return false;
}

bool inspector_color::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    auto& data = var.get_value<math::color>();
    if(ImGui::ColorEdit4("##",
                         math::value_ptr(data.value),
                         ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf))
    {
        return true;
    }

    return false;
}

bool inspector_quaternion::inspect(rtti::context& ctx,
                                   rttr::variant& var,
                                   const var_info& info,
                                   const meta_getter& get_metadata)
{
    auto data = var.get_value<math::quat>();

    auto val = quat_to_vec4(data);
    if(DragVec4(val, info))
    {
        data = vec4_to_quat(val);
        return true;
    }

    return false;
}

void inspector_transform::before_inspect(const rttr::property& prop)
{
    layout_ = std::make_unique<property_layout>(prop, false);
}

bool inspector_transform::inspect(rtti::context& ctx,
                                  rttr::variant& var,
                                  const var_info& info,
                                  const meta_getter& get_metadata)
{
    bool changed = false;

    auto data = var.get_value<math::transform>();
    auto translation = data.get_translation();
    auto rotation = data.get_rotation();
    auto scale = data.get_scale();
    auto skew = data.get_skew();
    auto perspective = data.get_perspective();

    static math::vec3 euler_angles(0.0f, 0.0f, 0.0f);

    math::quat old_quat(math::radians(euler_angles));

    float dotProduct = math::dot(old_quat, rotation);
    bool equal = (dotProduct > (1.0f - math::epsilon<float>()));
    if(!equal && (!ImGui::IsMouseDragging(ImGuiMouseButton_Left) || ImGuizmo::IsUsing()))
    {
        euler_angles = data.get_rotation_euler_degrees();
    }

    ImGui::PushID("Translation");
    {
        if(ImGui::Button("T", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            data.reset_translation();
            changed = true;
        }
        ImGui::SetItemTooltip("Translation");
        ImGui::SameLine();

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        {
            if(DragVec3(translation, info))
            {
                data.set_translation(translation);
                changed = true;
            }
        }
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    ImGui::PushID("Rotation");
    {
        if(ImGui::Button("R", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            data.reset_rotation();
            changed = true;
        }
        ImGui::SetItemTooltip("Rotation");

        ImGui::SameLine();

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        {
            auto old_euler = euler_angles;
            if(DragVec3(euler_angles, info))
            {
                data.rotate_local(math::radians(euler_angles - old_euler));
                changed = true;
            }
        }
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    ImGui::PushID("Scale");
    {
        if(ImGui::Button("S", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            data.reset_scale();
            changed = true;
        }
        ImGui::SetItemTooltip("Scale");

        ImGui::SameLine();

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        {
            if(DragVec3(scale, info))
            {
                data.set_scale(scale);
                changed = true;
            }
        }
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    ImGui::PushID("Skew");
    {
        if(ImGui::Button("S", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            data.reset_skew();
            changed = true;
        }
        ImGui::SetItemTooltip("Skew");

        ImGui::SameLine();

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        {
            if(DragVec3(skew, info))
            {
                data.set_skew(skew);
                changed = true;
            }
        }
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    ImGui::PushID("Perspective");
    {
        if(ImGui::Button("P", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight())))
        {
            data.reset_perspective();
            changed = true;
        }
        ImGui::SameLine();

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
        {
            if(DragVec4(perspective, info))
            {
                data.set_perspective(perspective);
                changed = true;
            }
        }
        ImGui::PopItemWidth();
    }
    ImGui::PopID();

    if(changed)
    {
        var = data;
    }

    return changed;
}
} // namespace ace
