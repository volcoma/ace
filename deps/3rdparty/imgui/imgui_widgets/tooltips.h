#pragma once
#include <imgui/imgui.h>

namespace ImGui
{
IMGUI_API void HelpMarker(const char* desc);

template<typename F>
void HelpMarker(const char* help, bool disabled, F&& f)
{
    if(disabled)
    {
        ImGui::TextDisabled("%s", help);

    }
    else
    {
        ImGui::Text("%s", help);
    }

    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        f();
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

IMGUI_API void ItemTooltip(const char* str, bool hover = true);

}
