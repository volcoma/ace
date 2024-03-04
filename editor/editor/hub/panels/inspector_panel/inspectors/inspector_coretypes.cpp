#include "inspector_coretypes.h"
#include <limits>
#include <filedialog/filedialog.h>

namespace ace
{
template<typename T>
bool inspect_scalar(rtti::context& ctx,
                    rttr::variant& var,
                    const var_info& info,
                    const inspector::meta_getter& get_metadata,
                    const char* format = nullptr)
{
    auto& data = var.get_value<T>();
    if(info.read_only)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(std::to_string(data).c_str());
    }
    else
    {
        T min{};
        T max{};
        float step{0.5};

        auto min_var = get_metadata("min");
        if(min_var)
            min = min_var.get_value<T>();

        auto max_var = get_metadata("max");
        if(max_var)
            max = max_var.get_value<T>();

        auto step_var = get_metadata("speed");
        if(step_var)
            step = step_var.get_value<float>();

        bool is_range = max_var.is_valid();

        if(is_range)
        {
            if(ImGui::SliderScalarT("##", &data, min, max, format))
            {
                return true;
            }
        }
        else
        {
            if(min_var)
                max = std::numeric_limits<T>::max();

            if(ImGui::DragScalarT("##", &data, step, min, max, format))
            {
                return true;
            }
        }
    }

    return false;
}

bool inspector_bool::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata)
{
    auto& data = var.get_value<bool>();

    if(info.read_only)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(data ? "true" : "false");
    }
    else
    {
        if(ImGui::Checkbox("##", &data))
        {
            return true;
        }
    }

    return false;
}

bool inspector_float::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    return inspect_scalar<float>(ctx, var, info, get_metadata);
}

bool inspector_double::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata)
{
    return inspect_scalar<double>(ctx, var, info, get_metadata);
}

bool inspector_int8::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata)
{
    return inspect_scalar<int8_t>(ctx, var, info, get_metadata);
}

bool inspector_int16::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    return inspect_scalar<int16_t>(ctx, var, info, get_metadata);
}

bool inspector_int32::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    return inspect_scalar<int32_t>(ctx, var, info, get_metadata);
}

bool inspector_int64::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    return inspect_scalar<int64_t>(ctx, var, info, get_metadata);
}

bool inspector_uint8::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    return inspect_scalar<uint8_t>(ctx, var, info, get_metadata);
}

bool inspector_uint16::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata)
{
    return inspect_scalar<uint16_t>(ctx, var, info, get_metadata);
}

bool inspector_uint32::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata)
{
    return inspect_scalar<uint32_t>(ctx, var, info, get_metadata);
}

bool inspector_uint64::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata)
{
    return inspect_scalar<uint64_t>(ctx, var, info, get_metadata);
}

bool inspector_string::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata)
{
    auto& data = var.get_value<std::string>();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

    if(info.read_only)
    {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    if(ImGui::InputTextWidget<128>("##", data, false, flags))
    {
        return true;
    }


    return false;
}

bool inspector_path::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata)
{
    auto& data = var.get_value<fs::path>();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

    if(info.read_only)
    {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    bool changed = false;

    std::string picked = data.generic_string();

    if(!info.read_only)
    {
        if(ImGui::Button(ICON_MDI_FOLDER_OPEN))
        {
            if(native::pick_folder_dialog(picked))
            {
                data = picked;
                picked = data.generic_string();
                changed = true;
            }
        }
        ImGui::SetItemTooltip("Pick a location...");
        ImGui::SameLine();
    }



    if(ImGui::InputTextWidget<256>("##", picked, false, flags))
    {
        data = picked;
        changed = true;
    }


    return changed;
}

bool inspector_duration_sec_float::inspect(rtti::context& ctx,
                                           rttr::variant& var,
                                           const var_info& info,
                                           const inspector::meta_getter& get_metadata)
{
    auto data = var.get_value<std::chrono::duration<float>>();
    auto count = data.count();
    rttr::variant v = count;

    bool changed = inspect_scalar<float>(ctx, v, info, get_metadata, "%.3fs");
    if(changed)
    {
        count = v.get_value<float>();
        var = std::chrono::duration<float>(count);
    }

    return changed;
}

bool inspector_duration_sec_double::inspect(rtti::context& ctx,
                                            rttr::variant& var,
                                            const var_info& info,
                                            const inspector::meta_getter& get_metadata)
{
    auto data = var.get_value<std::chrono::duration<double>>();
    auto count = data.count();
    rttr::variant v = count;
    bool changed = inspect_scalar<double>(ctx, v, info, get_metadata, "%.f3s");
    if(changed)
    {
        count = v.get_value<double>();
        var = std::chrono::duration<double>(count);
    }

    return changed;
}

bool inspector_uuid::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata)
{
    auto& data = var.get_value<hpp::uuid>();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

    if(info.read_only)
    {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    auto str = hpp::to_string(data);
    if(ImGui::InputTextWidget<128>("##", str, false, flags))
    {
        auto result = hpp::uuid::from_string(str);
        if(result)
        {
            data = result.value();
            return true;
        }
    }

    return false;
}
} // namespace ace
