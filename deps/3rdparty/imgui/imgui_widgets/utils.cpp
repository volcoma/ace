#include "utils.h"
#include "imgui/imgui.h"
#include <chrono>
#include <imgui/imgui_internal.h>

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ImGui
{

namespace
{

enum class size_fit
{
    shrink_to_fit,
    stretch_to_fit,
    auto_fit
};

enum class dimension_fit
{
    x,
    y,
    uniform,
    non_uniform
};

ImVec2 fit_item(float item_w, float item_h, float area_w, float area_h, size_fit sz_fit, dimension_fit dim_fit)
{
    float xscale = 1.0f;
    float yscale = 1.0f;

    item_w = std::max(item_w, 1.0f);
    item_h = std::max(item_h, 1.0f);

    switch(sz_fit)
    {
        case size_fit::shrink_to_fit:
        {
            if(item_w > area_w)
            {
                xscale = std::min(xscale, float(area_w) / item_w);
            }
            if(item_h > area_h)
            {
                yscale = std::min(yscale, float(area_h) / item_h);
            }
        }
        break;

        case size_fit::stretch_to_fit:
        {
            if(item_w < area_w)
            {
                xscale = std::max(xscale, float(area_w) / item_w);
            }
            if(item_h < area_h)
            {
                yscale = std::max(yscale, float(area_h) / item_h);
            }
        }
        break;

        case size_fit::auto_fit:
        {
            if(item_w > area_w)
            {
                xscale = std::min(xscale, float(area_w) / item_w);
            }
            else
            {
                xscale = std::max(xscale, float(area_w) / item_w);
            }

            if(item_h > area_h)
            {
                yscale = std::min(yscale, float(area_h) / item_h);
            }
            else
            {
                yscale = std::max(yscale, float(area_h) / item_h);
            }
        }
    }

    switch(dim_fit)
    {
        case dimension_fit::x:
            yscale = 1.0f;
            break;

        case dimension_fit::y:
            xscale = 1.0f;
            break;

        case dimension_fit::uniform:
        {
            float uniform_scale = std::min(xscale, yscale);
            xscale = uniform_scale;
            yscale = uniform_scale;
        }
        break;
        case dimension_fit::non_uniform:
            break;
    }

    return {xscale, yscale};
}

bool IsItemDisabled()
{
    return ImGui::GetItemFlags() & ImGuiItemFlags_Disabled;
}

ImRect RectExpanded(const ImRect& rect, float x, float y)
{
    ImRect result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

struct ImGuiDataTypeInfo
{
    size_t Size;
    const char* PrintFmt; // Unused
    const char* ScanFmt;
};

static const ImGuiDataTypeInfo typeinfos[] = {
    {sizeof(char), "%d", "%d"}, // ImGuiDataType_S8
    {sizeof(unsigned char), "%u", "%u"},
    {sizeof(short), "%d", "%d"}, // ImGuiDataType_S16
    {sizeof(unsigned short), "%u", "%u"},
    {sizeof(int), "%d", "%d"}, // ImGuiDataType_S32
    {sizeof(unsigned int), "%u", "%u"},
#ifdef _MSC_VER
    {sizeof(ImS64), "%I64d", "%I64d"}, // ImGuiDataType_S64
    {sizeof(ImU64), "%I64u", "%I64u"},
#else
    {sizeof(ImS64), "%lld", "%lld"}, // ImGuiDataType_S64
    {sizeof(ImU64), "%llu", "%llu"},
#endif
    {sizeof(float), "%f", "%f"},   // ImGuiDataType_Float (float are promoted to double in va_arg)
    {sizeof(double), "%f", "%lf"}, // ImGuiDataType_Double
};
IM_STATIC_ASSERT(IM_ARRAYSIZE(typeinfos) == ImGuiDataType_COUNT);
} // namespace

auto GetDataPrintFormat(ImGuiDataType data_type) -> const char*
{
    return typeinfos[data_type].PrintFmt;
}

bool DragMultiFormatScalarN(const char* label,
                            ImGuiDataType data_type,
                            void* p_data,
                            int components,
                            float v_speed,
                            const void* p_min,
                            const void* p_max,
                            const char** format,
                            ImGuiSliderFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if(window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components, CalcItemWidth());
    size_t type_size = typeinfos[data_type].Size;
    for(int i = 0; i < components; i++)
    {
        PushID(i);
        if(i > 0)
            SameLine(0, g.Style.ItemInnerSpacing.x);
        value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format[i], flags);
        DrawItemActivityOutline();

        PopID();
        PopItemWidth();
        p_data = (void*)((char*)p_data + type_size);
    }
    PopID();

    const char* label_end = FindRenderedTextEnd(label);
    if(label != label_end)
    {
        SameLine(0, g.Style.ItemInnerSpacing.x);
        TextEx(label, label_end);
    }

    EndGroup();
    return value_changed;
}

bool DragVecN(const char* label,
              ImGuiDataType data_type,
              void* p_data,
              int components,
              float v_speed,
              const void* p_min,
              const void* p_max,
              const void* p_default_data,
              const char* format,
              ImGuiSliderFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if(window->SkipItems)
        return false;

    std::array<const char*, 4> labels = {{"X", "Y", "Z", "W"}};
    std::array<ImColor, 4> colors = {
        {ImColor(125, 0, 0), ImColor(0, 125, 0), ImColor(0, 0, 125), ImColor(0, 125, 125)}};

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);

    auto w = CalcItemWidth();
    for(int i = 0; i < components; ++i)
    {
        const ImVec2 label_size = CalcTextSize(labels[i], NULL, true);
        float padded_size = label_size.x + GetStyle().FramePadding.x * 2.0f;
        w -= padded_size;
    }
    w -= GetStyle().ItemInnerSpacing.x * components;

    PushMultiItemsWidths(components, w);
    size_t type_size = typeinfos[data_type].Size;
    for(int i = 0; i < components; i++)
    {
        PushID(i);
        if(i > 0)
            SameLine(0, g.Style.ItemInnerSpacing.x);

        PushStyleColor(ImGuiCol_Button, colors[i].Value);
        if(Button(labels[i]))
        {
            value_changed = true;
            if(p_default_data)
            {
                memcpy(p_data, p_default_data, type_size);
            }
        }
        PopStyleColor();
        SameLine(0.0f, GetStyle().ItemInnerSpacing.x);

        value_changed |= DragScalar("", data_type, p_data, v_speed, p_min, p_max, format, flags);
        DrawItemActivityOutline();

        PopID();
        PopItemWidth();
        p_data = (void*)((char*)p_data + type_size);

        if(p_default_data)
        {
            p_default_data = (void*)((char*)p_default_data + type_size);
        }
    }
    PopID();

    const char* label_end = FindRenderedTextEnd(label);
    if(label != label_end)
    {
        SameLine(0, g.Style.ItemInnerSpacing.x);
        TextEx(label, label_end);
    }

    EndGroup();
    return value_changed;
}

void AlignedItem(float align, float totalWidth, float itemWidth, const std::function<void()>& itemDrawFn)
{
    float offset = totalWidth - itemWidth;
    float leftOffset = offset * align;
    float rightOffset = offset * (1.0f - align);

    auto oldSpacing = ImGui::GetStyle().ItemSpacing;
    BeginGroup();
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0, oldSpacing.y));
    if(leftOffset > 0.0f)
    {
        Dummy(ImVec2(leftOffset, 0));
        SameLine();
    }
    else
    {
        SetCursorPosX(GetCursorPosX() + leftOffset);
    }
    PopStyleVar();
    itemDrawFn();
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0, oldSpacing.y));
    if(rightOffset > 0.0f)
    {
        SameLine();
        Dummy(ImVec2(rightOffset, 0));
    }
    PopStyleVar();
    EndGroup();
}

std::string GetKeyCombinationName(const ImGuiKeyCombination& keys)
{
    std::string result{};
    for(size_t i = 0; i < keys.size(); ++i)
    {
        const auto& key = keys[i];
        result += GetKeyName(key);

        if(i + 1 < keys.size())
        {
            result += " + ";
        }
    }
    return result;
}

bool IsCombinationKeyPressed(const ImGuiKeyCombination& keys)
{
    for(size_t i = 0; i < keys.size(); ++i)
    {
        if(!IsKeyDown(keys[i]))
        {
            return false;
        }
    }

    for(size_t i = 0; i < keys.size(); ++i)
    {
        if(IsKeyPressed(keys[i], false))
        {
            return true;
        }
    }

    return false;
}

bool IsItemCombinationKeyPressed(const ImGuiKeyCombination& keys)
{
    if(IsWindowFocused())
    {
        // if(!IsAnyItemActive())
        {
            if(IsCombinationKeyPressed(keys))
            {
                return true;
            }
        }
    }

    return false;
}

bool IsItemDoubleClicked(ImGuiMouseButton mouse_button)
{
    return IsMouseDoubleClicked(mouse_button) && IsItemHovered(ImGuiHoveredFlags_None);
}

bool IsItemReleased(ImGuiMouseButton mouse_button)
{
    return IsMouseReleased(mouse_button) && IsItemHovered(ImGuiHoveredFlags_None);
}

bool IsItemKeyPressed(ImGuiKey key, bool repeat)
{
    bool result = false;
    if(IsWindowFocused())
    {
        if(!IsAnyItemActive())
        {
            if(IsKeyPressed(key, repeat))
            {
                result = true;
            }
        }
    }

    return result;
}
bool IsItemKeyReleased(ImGuiKey key)
{
    bool result = false;
    if(IsWindowFocused())
    {
        if(!IsAnyItemActive())
        {
            if(IsKeyReleased(key))
            {
                result = true;
            }
        }
    }

    return result;
}

void RenderFocusFrame(ImVec2 p_min, ImVec2 p_max, ImU32 color)
{
    ImGuiNavHighlightFlags flags = ImGuiNavHighlightFlags_None;

    ImGuiContext& g = *GetCurrentContext();
    ImGuiWindow* window = GetCurrentWindow();

    ImRect bb(p_min, p_max);

    float rounding = (flags & ImGuiNavHighlightFlags_NoRounding) ? 0.0f : g.Style.FrameRounding;
    ImRect display_rect = bb;
    // display_rect.ClipWith(window->ClipRect);
    const float thickness = 2.0f;
    // if (flags & ImGuiNavHighlightFlags_Compact)
    // {
    //     display_rect.ClipWithFull(window->ClipRect);
    //     if(!window->ClipRect.Overlaps(display_rect))
    //     {
    //         return;
    //     }
    window->DrawList->AddRect(display_rect.Min, display_rect.Max, color, rounding, 0, thickness);
    // }
    // else
    // {
    //     const float distance = 3.0f + thickness * 0.5f;
    //     display_rect.Expand(ImVec2(distance, distance));

    //     display_rect.ClipWithFull(window->ClipRect);
    //     if(!window->ClipRect.Overlaps(display_rect))
    //     {
    //         return;
    //     }
    //     bool fully_visible = window->ClipRect.Contains(display_rect);
    //     if(!fully_visible)
    //         window->DrawList->PushClipRect(display_rect.Min, display_rect.Max);
    //     window->DrawList->AddRect(display_rect.Min, display_rect.Max, color, rounding, 0, thickness);

    //     if(!fully_visible)
    //         window->DrawList->PopClipRect();
    // }
}

void SetItemFocusFrame(ImU32 color)
{
    RenderFocusFrame(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), color);
}

void SameLineInner()
{
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
}

void RenderFrameEx(ImVec2 p_min, ImVec2 p_max, float rounding, float thickness)
{
    ImGuiWindow* window = GetCurrentWindow();

    if(rounding < 0)
    {
        rounding = ImGui::GetStyle().FrameRounding;
    }

    window->DrawList->AddRect(p_min + ImVec2(1, 1),
                              p_max + ImVec2(1, 1),
                              GetColorU32(ImGuiCol_BorderShadow),
                              rounding,
                              0,
                              thickness);
    window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, 0, thickness);
}

void Spinner(float radius, float thickness, int num_segments, float speed, ImU32 color)
{
    auto window = GetCurrentWindow();
    if(window->SkipItems)
    {
        return;
    }

    auto& g = *ImGui::GetCurrentContext();
    const auto pos = window->DC.CursorPos;

    ImVec2 size{radius * 2, radius * 2};
    const ImRect bb{pos, pos + size};
    ItemSize(bb);
    if(!ItemAdd(bb, 0))
    {
        return;
    }

    auto time = static_cast<float>(g.Time) * speed;
    window->DrawList->PathClear();
    int start = static_cast<int>(abs(ImSin(time) * (num_segments - 5)));
    const float a_min = IM_PI * 2.0f * float(start) / float(num_segments);
    const float a_max = IM_PI * 2.0f * float(num_segments - 3) / float(num_segments);
    auto centre = pos;
    centre.x += radius;
    centre.y += radius;
    for(auto i = 0; i < num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(
            {centre.x + ImCos(a + time * 8) * radius, centre.y + ImSin(a + time * 8) * radius});
    }
    window->DrawList->PathStroke(GetColorU32(color), false, thickness);
}

void ImageWithAspect(ImTextureID texture,
                     ImVec2 texture_size,
                     ImVec2 size,
                     const ImVec2& uv0,
                     const ImVec2& uv1,
                     const ImVec4& tint_col,
                     const ImVec4& border_col)
{
    auto scale =
        fit_item(texture_size.x, texture_size.y, size.x, size.y, size_fit::shrink_to_fit, dimension_fit::uniform);

    texture_size = texture_size * scale;

    AlignedItem(0.5f,
                size.x,
                texture_size.x,
                [&]()
                {
                    Image(texture, texture_size, uv0, uv1, tint_col, border_col);
                });
}

bool ContentButtonItem(const ContentItem& item)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems)
        return false;

    ImVec2 size = item.image_size;
    if(size.x <= 0 && size.y <= 0)
    {
        size.x = size.y = ImGui::GetTextLineHeightWithSpacing();
    }
    else
    {
        if(size.x <= 0)
            size.x = size.y;
        else if(size.y <= 0)
            size.y = size.x;
    }

    ImGuiContext& g = *ImGui::GetCurrentContext();
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(item.name);

    if(item.name_font)
    {
        ImGui::PushFont(item.name_font);
    }
    ImVec2 textSize{};

    if(item.name)
    {
        textSize = ImGui::CalcTextSize(item.name, nullptr, true);
    }
    if(item.name_font)
    {
        ImGui::PopFont();
    }

    if(item.type_font)
    {
        ImGui::PushFont(item.type_font);
    }
    ImVec2 typeSize{};

    if(item.type)
    {
        typeSize = ImGui::CalcTextSize(item.type, nullptr, true);
    }
    if(item.type_font)
    {
        ImGui::PopFont();
    }
    ImVec2 textPadding(6.0f, style.ItemInnerSpacing.y * 2.0f);

    ImVec2 padding = {0.0f, 0.0f};

    if(textSize.x < 1.0f)
    {
        padding = {};
        textPadding = {};
        textSize.y = {};
    }

    if(typeSize.x < 1.0f)
    {
        typeSize.y = {};
    }

    ImVec2 totalSize(size.x, size.y + textSize.y + typeSize.y + textPadding.y);

    ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSize + padding * 2);
    ImVec2 start = window->DC.CursorPos + padding;

    ImRect image_bb(start, start + size);
    image_bb.Expand(-2.0f);

    ItemSize(bb);
    if(!ItemAdd(bb, id))
        return false;

    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive
                                  : hovered         ? ImGuiCol_ButtonHovered
                                                    : ImGuiCol_Button);

    // Fit the texture in the bounding box.
    auto imgSz = item.texture_size;
    const auto fittingBoxSize = ImVec2(image_bb.GetWidth(), image_bb.GetHeight());

    auto scale =
        fit_item(imgSz.x, imgSz.y, fittingBoxSize.x, fittingBoxSize.y, size_fit::shrink_to_fit, dimension_fit::uniform);
    imgSz *= scale;

    image_bb.Min.x += (fittingBoxSize.x - imgSz.x) * 0.5f;
    image_bb.Min.y += (fittingBoxSize.y - imgSz.y) * 0.5f;

    image_bb.Max = image_bb.Min + imgSz;

    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if(item.bg_col.w > 0.0f)
        window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(item.bg_col), style.FrameRounding);

    window->DrawList->AddImageRounded(item.texId,
                                      image_bb.Min,
                                      image_bb.Max,
                                      item.uv0,
                                      item.uv1,
                                      GetColorU32(item.tint_col),
                                      style.FrameRounding);

    if(textSize.x > 0)
    {
        start.x += textPadding.x;
        totalSize.x -= 2.0f * textPadding.x;

        start.y += fittingBoxSize.y + style.ItemInnerSpacing.y;

        auto originalStart = start;
        if(totalSize.x > textSize.x)
        {
            start.x += (totalSize.x - textSize.x) * 0.5f;
        }

        if(item.name_font)
        {
            ImGui::PushFont(item.name_font);
        }

        auto end = start + ImVec2(totalSize.x - ImGui::CalcTextSize("...").x, textSize.y);
        ImGui::RenderTextEllipsis(window->DrawList,
                                  start,
                                  end,
                                  start.x + totalSize.x,
                                  start.x + totalSize.x,
                                  item.name,
                                  nullptr,
                                  &textSize);

        if(item.name_font)
        {
            ImGui::PopFont();
        }

        if(item.type_font)
        {
            ImGui::PushFont(item.type_font);
        }

        start = originalStart;
        start.y += textSize.y + style.ItemInnerSpacing.y;

        if(totalSize.x > typeSize.x)
        {
            start.x += (totalSize.x - typeSize.x) * 0.5f;
        }

        end = start + ImVec2(totalSize.x - ImGui::CalcTextSize("...").x, typeSize.y);

        ImGui::RenderTextEllipsis(window->DrawList,
                                  start,
                                  end,
                                  start.x + totalSize.x,
                                  start.x + totalSize.x,
                                  item.type,
                                  nullptr,
                                  &typeSize);

        if(item.type_font)
        {
            ImGui::PopFont();
        }
    }
    return pressed;
}

ImVec2 CalcItemSize(const char* label, ImVec2 size_arg)
{
    const auto& style = GetStyle();
    const auto label_size = CalcTextSize(label, NULL, true);
    auto size =
        CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    return size;
}

void ItemBrowser(float item_width, size_t items_count, const std::function<void(int)>& callback)
{
    const auto& style = GetStyle();

    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

    auto avail = GetContentRegionAvail().x;
    // add one extra item spacing because we are adding it for every item, but should not for the last one on the line
    avail += style.ItemSpacing.x;
    auto item_size = item_width + style.ItemSpacing.x;
    auto items_per_line_exact = avail / item_size;
    auto items_per_line_floor = ImMax(1.0f, ImFloor(items_per_line_exact));
    auto items_per_line = ImMin(size_t(items_per_line_floor), items_count);
    auto extra = ((items_per_line_exact - items_per_line_floor) * item_size) / ImMax(1.0f, items_per_line_floor - 1);

    if(float(items_count) < items_per_line_exact)
    {
        extra = {};
    }
    auto lines = items_per_line > 0 ? int(ImCeil(float(items_count) / float(items_per_line))) : 0;
    ImGuiListClipper clipper;
    clipper.Begin(lines);

    while(clipper.Step())
    {
        for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            auto start = size_t(i) * items_per_line;
            auto end = start + ImMin(items_count - start, items_per_line);
            for(size_t j = start; j < end; ++j)
            {
                PushID(int(j));

                callback(j);

                PopID();

                if(j != end - 1)
                {
                    SameLine(0.0f, style.ItemSpacing.x + extra);
                }
            }
        }
    }
    PopStyleVar();
}

bool ImageMenuItem(ImTextureID texture, const char* tooltip, bool selected, bool enabled)
{
    ImVec4 bg_color(0, 0, 0, 0);

    ImVec2 size(GetTextLineHeight(), GetTextLineHeight());
    bool ret = false;

    {
        ImVec4 tintColor(1.0f, 1.0f, 1.0f, 1.0f);

        if(!enabled)
        {
            PushItemFlag(ImGuiItemFlags_Disabled, true);
            tintColor = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
        }

        if(ImageButton(texture, size, ImVec2(0, 0), ImVec2(1, 1), -1, bg_color, tintColor))
        {
            ret = true;
        }

        if(!enabled)
        {
            PopItemFlag();
        }
    }
    if(tooltip && IsItemHovered())
    {
        SetTooltip("%s", tooltip);
    }

    if(selected)
    {
        ImVec2 rectMin = GetItemRectMin();
        ImVec2 rectMax = GetItemRectMax();
        RenderFocusFrame(rectMin, rectMax, ImColor(ImVec4(1.0f, 0.6f, 0.0f, 1.0f)));
    }

    return ret;
}

WindowTimeBlock::WindowTimeBlock(ImFont* font)
{
    start_ = clock_t::now();
    font_ = font;
}

WindowTimeBlock::~WindowTimeBlock()
{
    using duration_t = std::chrono::duration<float, std::milli>;

    auto end = clock_t::now();
    auto dur = std::chrono::duration_cast<duration_t>(end - start_);

    char text[32];
    ImFormatString(text, IM_ARRAYSIZE(text), "%.3fms", dur.count());

    ImGui::PushFont(font_);
    auto textSize = ImGui::CalcTextSize(text);

    auto windowPos = ImGui::GetWindowPos();
    auto windowSize = ImGui::GetWindowSize();

    auto textPos = windowPos + windowSize - textSize - ImGui::GetStyle().WindowPadding;
    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), text);
    ImGui::PopFont();
}

bool IsDragDropPossibleTargetForType(const char* type)
{
    auto testPaylopad = ImGui::GetDragDropPayload();
    {
        if(testPaylopad && testPaylopad->IsDataType(type))
        {
            return true;
        }
    }

    return false;
}

void DrawItemActivityOutline(OutlineFlags flags, ImColor colourHighlight, float rounding)
{
    if(IsItemDisabled())
        return;

    auto* drawList = ImGui::GetWindowDrawList();
    ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    rect = RectExpanded(rect, -0.5f, -0.5f);

    if(rounding < 0.0f)
    {
        rounding = ImGui::GetStyle().FrameRounding;
    }
    if((flags & OutlineFlags_WhenActive) && ImGui::IsItemActive())
    {
        if(flags & OutlineFlags_HighlightActive)
        {
            drawList->AddRect(rect.Min, rect.Max, colourHighlight, rounding, 0, 1.5f);
        }
        else
        {
            drawList->AddRect(rect.Min, rect.Max, ImColor(60, 60, 60), rounding, 0, 1.5f);
        }
    }
    else if((flags & OutlineFlags_WhenHovered) && ImGui::IsItemHovered() && !ImGui::IsItemActive())
    {
        drawList->AddRect(rect.Min, rect.Max, ImColor(60, 60, 60), rounding, 0, 1.5f);
    }
    else if((flags & OutlineFlags_WhenInactive) && !ImGui::IsItemHovered() && !ImGui::IsItemActive())
    {
        drawList->AddRect(rect.Min, rect.Max, ImColor(50, 50, 50), rounding, 0, 1.0f);
    }
}

void DrawFilterWithHint(ImGuiTextFilter& filter, const char* hint_text, float width)
{
    // Start an input text with filter
    ImGui::PushID(&filter);
    ImGui::SetNextItemWidth(width);

    if(ImGui::InputText("##Filter", filter.InputBuf, IM_ARRAYSIZE(filter.InputBuf), ImGuiInputTextFlags_AutoSelectAll))
    {
        filter.Build();
    }
    ImGui::PopID();

    // Check if the filter text is empty
    if(filter.InputBuf[0] == '\0' && !ImGui::IsItemActive())
    {
        auto offset = ImGui::GetStyle().FramePadding.x;
        // Draw the hint text
        ImVec2 pos = ImGui::GetItemRectMin();
        pos.x += offset;
        ImVec2 size = ImGui::GetItemRectSize();
        size.x -= 2.0f * offset;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f)); // Set hint text color
        ImGui::RenderTextClipped(pos,
                                 ImVec2(pos.x + size.x, pos.y + size.y),
                                 hint_text,
                                 nullptr,
                                 nullptr,
                                 ImVec2(0.0f, 0.5f));
        ImGui::PopStyleColor();
    }
}

void WrapMousePos(int axises_mask, const ImVec2& wrap_rect_min, const ImVec2& wrap_rect_max)
{
    ImRect wrap_rect(wrap_rect_min, wrap_rect_max);
    ImGuiContext& g = *GImGui;
    IM_ASSERT(axises_mask == 1 || axises_mask == 2 || axises_mask == (1 | 2));
    ImVec2 p_mouse = g.IO.MousePos;
    for(int axis = 0; axis < 2; axis++)
    {
        if((axises_mask & (1 << axis)) == 0)
            continue;
        float size = wrap_rect.Max[axis] - wrap_rect.Min[axis];
        if(p_mouse[axis] >= wrap_rect.Max[axis])
            p_mouse[axis] = wrap_rect.Min[axis] + 1.0f;
        else if(p_mouse[axis] <= wrap_rect.Min[axis])
            p_mouse[axis] = wrap_rect.Max[axis] - 1.0f;
    }
    if(p_mouse.x != g.IO.MousePos.x || p_mouse.y != g.IO.MousePos.y)
        TeleportMousePos(p_mouse);
}

void WrapMousePos(int axises_mask)
{
    ImGuiContext& g = *GetCurrentContext();
#ifdef IMGUI_HAS_DOCK
    if(g.IO.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        const ImGuiPlatformMonitor* monitor = GetViewportPlatformMonitor(g.MouseViewport);
        WrapMousePos(axises_mask, monitor->MainPos, monitor->MainPos + monitor->MainSize - ImVec2(1, 1));
    }
    else
#endif
    {
        ImGuiViewport* viewport = GetMainViewport();
        WrapMousePos(axises_mask, viewport->Pos, viewport->Pos + viewport->Size - ImVec2(1, 1));
    }
}

void WrapMousePos()
{
    WrapMousePos(1 << ImGuiAxis_X | 1 << ImGuiAxis_Y);
}

void ActiveItemWrapMousePos()
{
    ImGuiContext& g = *GetCurrentContext();
    ImGuiID id = GetItemID();

    if (IsItemActive() && (!GetInputTextState(id) || g.InputTextDeactivatedState.ID == id))
    {
        WrapMousePos(1 << ImGuiAxis_X);
    }
}

void ActiveItemWrapMousePos(const ImVec2& wrap_rect_min, const ImVec2& wrap_rect_max)
{
    ImGuiContext& g = *GetCurrentContext();
    ImGuiID id = GetItemID();

    if (IsItemActive() && (!GetInputTextState(id) || g.InputTextDeactivatedState.ID == id))
    {
        WrapMousePos(1 << ImGuiAxis_X, wrap_rect_min, wrap_rect_max);
    }
}

bool BeginPopupContextWindowEx(const char* str_id, ImGuiPopupFlags popup_flags)
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = g.CurrentWindow;
    if(!str_id)
        str_id = "window_context";
    ImGuiID id = window->GetID(str_id);

    auto pos = ImGui::GetWindowPos();
    auto size = ImGui::GetWindowSize();
    ImRect r(pos, pos + size);

    int mouse_button = (popup_flags & ImGuiPopupFlags_MouseButtonMask_);
    if(r.Contains(ImGui::GetIO().MouseClickedPos[mouse_button]) && ImGui::IsMouseReleased(mouse_button) &&
       ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
        if(!(popup_flags & ImGuiPopupFlags_NoOpenOverItems) || !ImGui::IsAnyItemHovered())
            ImGui::OpenPopupEx(id, popup_flags);
    return ImGui::BeginPopupEx(id,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoSavedSettings);
}

} // namespace ImGui
