#include "tooltips.h"

namespace ImGui
{
void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) && ImGui::BeginTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ItemTooltip(const char* str, bool hover)
{
    if(hover)
    {
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {
            ImGui::BeginTooltip();
            {
                ImGui::TextUnformatted(str);
            }
            ImGui::EndTooltip();
        }
    }
    else
    {
        auto sz = ImGui::GetItemRectSize();
        ImVec2 tooltip_pos = ImGui::GetItemRectMin() + ImVec2(0.0f, sz.y);
        auto old_pos = ImGui::GetIO().MousePos;
        ImGui::GetIO().MousePos = tooltip_pos;

        ImGuiWindowFlags flags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoNav;
        ImGui::Begin(str, nullptr, flags);
        {
            ImGui::TextUnformatted(str);
        }
        ImGui::End();
        ImGui::GetIO().MousePos = old_pos;
    }
}

} // namespace ImGui
