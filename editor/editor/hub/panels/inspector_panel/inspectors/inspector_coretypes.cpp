#include "inspector_coretypes.h"
#include <filedialog/filedialog.h>
#include <limits>

namespace ace
{
template<typename T>
auto inspect_scalar(rtti::context& ctx,
                    rttr::variant& var,
                    const var_info& info,
                    const inspector::meta_getter& get_metadata,
                    const char* format = nullptr) -> inspect_result
{
    inspect_result result{};

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

        auto step_var = get_metadata("step");
        if(step_var)
            step = step_var.get_value<float>();

        bool is_range = max_var.is_valid();

        if(is_range)
        {
            if(std::is_floating_point_v<T>)
            {
                if(format == nullptr)
                {
                    if(step < 0.001)
                    {
                        format = "%.4f";
                    }
                    if(step < 0.0001)
                    {
                        format = "%.5f";
                    }
                }
            }

            result.changed = ImGui::SliderScalarT("##", &data, min, max, format);
            result.edit_finished = ImGui::IsItemDeactivatedAfterEdit();
            ImGui::DrawItemActivityOutline();
        }
        else
        {
            if(min_var)
                max = std::numeric_limits<T>::max();

            result.changed = ImGui::DragScalarT("##", &data, step, min, max, format);
            result.edit_finished = ImGui::IsItemDeactivatedAfterEdit();
            ImGui::DrawItemActivityOutline();
        }
    }

    return result;
}

auto inspector_bool::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata) -> inspect_result
{
    auto& data = var.get_value<bool>();
    inspect_result result{};

    if(info.read_only)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(data ? "true" : "false");
    }
    else
    {
        result.changed = ImGui::Checkbox("##", &data);
        result.edit_finished = ImGui::IsItemDeactivatedAfterEdit();

        ImGui::DrawItemActivityOutline();
    }

    return result;
}

auto inspector_float::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<float>(ctx, var, info, get_metadata);
}

auto inspector_double::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<double>(ctx, var, info, get_metadata);
}

auto inspector_int8::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<int8_t>(ctx, var, info, get_metadata);
}

auto inspector_int16::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<int16_t>(ctx, var, info, get_metadata);
}

auto inspector_int32::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<int32_t>(ctx, var, info, get_metadata);
}

auto inspector_int64::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<int64_t>(ctx, var, info, get_metadata);
}

auto inspector_uint8::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<uint8_t>(ctx, var, info, get_metadata);
}

auto inspector_uint16::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<uint16_t>(ctx, var, info, get_metadata);
}

auto inspector_uint32::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<uint32_t>(ctx, var, info, get_metadata);
}

auto inspector_uint64::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata) -> inspect_result
{
    return inspect_scalar<uint64_t>(ctx, var, info, get_metadata);
}

auto inspector_string::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata) -> inspect_result
{
    auto& data = var.get_value<std::string>();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;

    if(info.read_only)
    {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    inspect_result result{};
    result.changed |= ImGui::InputTextWidget<128>("##", data, false, flags);
    result.edit_finished |= ImGui::IsItemDeactivatedAfterEdit();
    ImGui::DrawItemActivityOutline();
    return result;
}

auto inspector_path::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata) -> inspect_result
{
    auto& data = var.get_value<fs::path>();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;

    if(info.read_only)
    {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    inspect_result result{};

    std::string picked = data.generic_string();

    if(!info.read_only)
    {
        if(ImGui::Button(ICON_MDI_FOLDER_OPEN))
        {
            if(native::pick_folder_dialog(picked))
            {
                data = picked;
                picked = data.generic_string();
                result.changed = true;
                result.edit_finished |= true;
            }
        }
        ImGui::SetItemTooltip("Pick a location...");
        ImGui::SameLine();
    }

    result.changed |= ImGui::InputTextWidget<256>("##", picked, false, flags);
    result.edit_finished |= ImGui::IsItemDeactivatedAfterEdit();
    if(result.edit_finished)
    {
        data = picked;
        result.changed = true;
    }

    ImGui::DrawItemActivityOutline();

    return result;
}

auto inspector_duration_sec_float::inspect(rtti::context& ctx,
                                           rttr::variant& var,
                                           const var_info& info,
                                           const inspector::meta_getter& get_metadata) -> inspect_result
{
    auto data = var.get_value<std::chrono::duration<float>>();
    auto count = data.count();
    rttr::variant v = count;

    auto result = inspect_scalar<float>(ctx, v, info, get_metadata, "%.3fs");
    if(result.changed)
    {
        count = v.get_value<float>();
        var = std::chrono::duration<float>(count);
    }

    return result;
}

auto inspector_duration_sec_double::inspect(rtti::context& ctx,
                                            rttr::variant& var,
                                            const var_info& info,
                                            const inspector::meta_getter& get_metadata) -> inspect_result
{
    auto data = var.get_value<std::chrono::duration<double>>();
    auto count = data.count();
    rttr::variant v = count;
    auto result = inspect_scalar<double>(ctx, v, info, get_metadata, "%.f3s");
    if(result.changed)
    {
        count = v.get_value<double>();
        var = std::chrono::duration<double>(count);
    }

    return result;
}

auto inspector_uuid::inspect(rtti::context& ctx,
                             rttr::variant& var,
                             const var_info& info,
                             const meta_getter& get_metadata) -> inspect_result
{
    auto& data = var.get_value<hpp::uuid>();

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll;

    if(info.read_only)
    {
        flags |= ImGuiInputTextFlags_ReadOnly;
    }

    auto str = hpp::to_string(data);

    inspect_result result{};

    result.changed = ImGui::InputTextWidget<128>("##", str, false, flags);
    result.edit_finished = ImGui::IsItemDeactivatedAfterEdit();
    ImGui::DrawItemActivityOutline();

    if(result.edit_finished)
    {
        auto parse_result = hpp::uuid::from_string(str);
        if(parse_result)
        {
            data = parse_result.value();
            result.changed = true;
            return result;
        }
    }

    result = {};
    return result;
}
} // namespace ace
