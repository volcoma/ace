#include "inspector_basetypes.h"
#include "imgui_widgets/utils.h"

namespace ace
{
template<typename T>
auto inspect_range_scalar(rtti::context& ctx,
                          rttr::variant& var,
                          const var_info& info,
                          const inspector::meta_getter& get_metadata) -> inspect_result
{
    auto& data = var.get_value<range<T>>();

    T min{};
    T max{};

    T* min_ptr{};
    T* max_ptr{};
    auto min_var = get_metadata("min");
    if(min_var)
    {
        min = min_var.get_value<T>();
        min_ptr = &min;
    }

    auto max_var = get_metadata("max");
    if(max_var)
    {
        max = max_var.get_value<T>();
        max_ptr = &max;
    }

    static const std::string min_fmt = std::string("Min:") + ImGui::GetDataPrintFormat<T>();
    static const std::string max_fmt = std::string("Max:") + ImGui::GetDataPrintFormat<T>();

    std::array<const char*, 2> formats = {{min_fmt.c_str(), min_fmt.c_str()}};

    inspect_result result{};
    result.changed = ImGui::DragMultiFormatScalarN("##",
                                                   ImGui::GetDataType<T>(),
                                                   &data.min,
                                                   2,
                                                   1.0f,
                                                   min_ptr,
                                                   max_ptr,
                                                   formats.data());

    result.edit_finished = ImGui::IsItemDeactivatedAfterEdit();

    //ImGui::DrawItemActivityOutline();

    return result;
}

auto inspector_range_float::inspect(rtti::context& ctx,
                                    rttr::variant& var,
                                    const var_info& info,
                                    const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<float>(ctx, var, info, get_metadata);
}

auto inspector_range_double::inspect(rtti::context& ctx,
                                     rttr::variant& var,
                                     const var_info& info,
                                     const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<double>(ctx, var, info, get_metadata);
}

auto inspector_range_int8::inspect(rtti::context& ctx,
                                   rttr::variant& var,
                                   const var_info& info,
                                   const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<int8_t>(ctx, var, info, get_metadata);
}

auto inspector_range_int16::inspect(rtti::context& ctx,
                                    rttr::variant& var,
                                    const var_info& info,
                                    const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<int16_t>(ctx, var, info, get_metadata);
}

auto inspector_range_int32::inspect(rtti::context& ctx,
                                    rttr::variant& var,
                                    const var_info& info,
                                    const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<int32_t>(ctx, var, info, get_metadata);
}

auto inspector_range_int64::inspect(rtti::context& ctx,
                                    rttr::variant& var,
                                    const var_info& info,
                                    const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<int64_t>(ctx, var, info, get_metadata);
}

auto inspector_range_uint8::inspect(rtti::context& ctx,
                                    rttr::variant& var,
                                    const var_info& info,
                                    const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<uint8_t>(ctx, var, info, get_metadata);
}

auto inspector_range_uint16::inspect(rtti::context& ctx,
                                     rttr::variant& var,
                                     const var_info& info,
                                     const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<uint16_t>(ctx, var, info, get_metadata);
}

auto inspector_range_uint32::inspect(rtti::context& ctx,
                                     rttr::variant& var,
                                     const var_info& info,
                                     const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<uint32_t>(ctx, var, info, get_metadata);
}

auto inspector_range_uint64::inspect(rtti::context& ctx,
                                     rttr::variant& var,
                                     const var_info& info,
                                     const meta_getter& get_metadata) -> inspect_result
{
    return inspect_range_scalar<uint64_t>(ctx, var, info, get_metadata);
}

} // namespace ace
