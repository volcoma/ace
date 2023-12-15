#pragma once
#include <array>
#include <cstring>
#include <functional>
#include <imgui/imgui.h>
#include <string>
#include <vector>

enum ImGuiMouseCursorEx_
{
    ImGuiMouseCursor_Help = ImGuiMouseCursor_COUNT,
    ImGuiMouseCursor_Wait,
    ImGuiMouseCursor_ArrowWait,
    ImGuiMouseCursor_Cross,
    ImGuiMouseCursorEx_COUNT
};

using ImGuiKeyCombination = std::vector<ImGuiKey>;
namespace ImGui
{

IMGUI_API bool DragMultiFormatScalarN(const char* label,
                                      ImGuiDataType data_type,
                                      void* p_data,
                                      int components,
                                      float v_speed = 1.0f,
                                      const void* p_min = NULL,
                                      const void* p_max = NULL,
                                      const char** format = NULL,
                                      ImGuiSliderFlags flags = 0);

IMGUI_API bool DragVecN(const char* label,
                        ImGuiDataType data_type,
                        void* p_data,
                        int components,
                        float v_speed = 1.0f,
                        const void* p_min = NULL,
                        const void* p_max = NULL,
                        const char* format = NULL,
                        ImGuiSliderFlags flags = 0);

IMGUI_API void AlignedItem(float align, float totalWidth, float itemWidth, const std::function<void()>& itemDrawFn);

IMGUI_API ImVec2 CalcItemSize(const char* label, ImVec2 size_arg = ImVec2(0, 0));

IMGUI_API std::string GetKeyCombinationName(const ImGuiKeyCombination& keys);
IMGUI_API bool IsItemDoubleClicked(ImGuiMouseButton mouse_button);
IMGUI_API bool IsItemReleased(ImGuiMouseButton mouse_button);
IMGUI_API bool IsItemKeyPressed(ImGuiKey key,
                                bool repeat = true); // was key pressed (went from !Down to Down)? if repeat=true, uses
                                                     // io.KeyRepeatDelay / KeyRepeatRate
IMGUI_API bool IsItemKeyReleased(ImGuiKey key);

IMGUI_API void RenderFocusFrame(ImVec2 p_min, ImVec2 p_max, ImU32 color = GetColorU32(ImGuiCol_NavHighlight));

IMGUI_API void Spinner(float radius,
                       float thickness,
                       int num_segments,
                       float speed,
                       ImU32 color = GetColorU32(ImGuiCol_NavHighlight));

IMGUI_API void ImageWithAspect(ImTextureID texture,
                               ImVec2 texture_size,
                               ImVec2 size,
                               const ImVec2& uv0 = ImVec2(0, 0),
                               const ImVec2& uv1 = ImVec2(1, 1),
                               const ImVec4& tint_col = ImVec4(1, 1, 1, 1),
                               const ImVec4& border_col = ImVec4(0, 0, 0, 0));

IMGUI_API bool ImageButtonWithAspectAndTextBelow(ImTextureID texId,
                                                 const std::string& name,
                                                 const ImVec2& texture_size,
                                                 const ImVec2& imageSize,
                                                 const ImVec2& uv0 = ImVec2(0, 0),
                                                 const ImVec2& uv1 = ImVec2(1, 1),
                                                 int frame_padding = -1,
                                                 const ImVec4& bg_col = ImVec4(0, 0, 0, 0),
                                                 const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

IMGUI_API bool ImageMenuItem(ImTextureID texture, const char* tooltip, bool selected = false, bool enabled = true);

template<size_t BuffSize = 64>
bool InputTextWidget(const std::string& inputFieldName,
                     std::string& source,
                     bool multiline = false,
                     ImGuiInputTextFlags flags = 0)
{
    std::array<char, BuffSize> buffer{{}};
    std::memcpy(buffer.data(), source.data(), std::min(buffer.size() - 1, source.size()));

    if(multiline)
    {
        if(ImGui::InputTextMultiline(inputFieldName.c_str(), buffer.data(), buffer.size(), ImVec2(0, 0), flags))
        {
            source = buffer.data();
            return true;
        }
    }
    else
    {
        if(ImGui::InputText(inputFieldName.c_str(), buffer.data(), buffer.size(), flags))
        {
            source = buffer.data();
            return true;
        }
    }

    return false;
}

template<typename T>
constexpr inline auto GetDataType() -> ImGuiDataType
{
    if(std::is_same<T, float>::value)
        return ImGuiDataType_Float;
    if(std::is_same<T, double>::value)
        return ImGuiDataType_Double;
    if(std::is_same<T, int8_t>::value)
        return ImGuiDataType_S8;
    if(std::is_same<T, int16_t>::value)
        return ImGuiDataType_S16;
    if(std::is_same<T, int32_t>::value)
        return ImGuiDataType_S32;
    if(std::is_same<T, int64_t>::value)
        return ImGuiDataType_S64;
    if(std::is_same<T, uint8_t>::value)
        return ImGuiDataType_U8;
    if(std::is_same<T, uint16_t>::value)
        return ImGuiDataType_U16;
    if(std::is_same<T, uint32_t>::value)
        return ImGuiDataType_U32;
    if(std::is_same<T, uint64_t>::value)
        return ImGuiDataType_U64;
    return ImGuiDataType_COUNT;
}

IMGUI_API const char* GetDataPrintFormat(ImGuiDataType data_type);

template<typename T>
constexpr inline auto GetDataPrintFormat() -> const char*
{
    return GetDataPrintFormat(GetDataType<T>());
}

template<typename T>
IMGUI_API bool DragScalarT(const char* label,
                           T* p_data,
                           float v_speed = 1.0f,
                           T p_min = {},
                           T p_max = {},
                           const char* format = NULL,
                           ImGuiSliderFlags flags = 0)
{
    return DragScalar(label, GetDataType<T>(), p_data, v_speed, &p_min, &p_max, format, flags);
}

template<typename T>
IMGUI_API bool SliderScalarT(const char* label,
                             T* p_data,
                             T p_min,
                             T p_max,
                             const char* format = NULL,
                             ImGuiSliderFlags flags = 0)
{
    return SliderScalar(label, GetDataType<T>(), p_data, &p_min, &p_max, format, flags);
}

IMGUI_API void ItemBrowser(float item_width, size_t items_count, const std::function<void(int index)>& callback);

} // namespace ImGui
