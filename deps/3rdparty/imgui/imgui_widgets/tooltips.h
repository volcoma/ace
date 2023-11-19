#pragma once
#include <imgui/imgui.h>

namespace ImGui
{
void HelpMarker(const char* desc);

template<typename F>
void HelpMarker(F&& f)
{
    ImGui::TextDisabled("(?)");

    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
//        ImGui::TextUnformatted(desc);
        f();
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

}
