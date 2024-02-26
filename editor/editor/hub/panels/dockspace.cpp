#include "dockspace.h"
#include "panels_defs.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <vector>

namespace ace
{

namespace
{
void build_dockspace(ImGuiID dockspace_id/*, ImVec2 size*/)
{
    ImGui::DockBuilderRemoveNode(dockspace_id); // Clear out existing layout

    const ImGuiDockNodeFlags dockNodeFlags = ImGuiDockNodeFlags_None;
    ImGui::DockBuilderAddNode(dockspace_id, dockNodeFlags); // Add empty node

    ImGuiID dock_main_id = dockspace_id;
    ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, nullptr, &dock_main_id);
    ImGuiID dock_right_down_id =
        ImGui::DockBuilderSplitNode(dock_right_id, ImGuiDir_Down, 0.3f, nullptr, &dock_right_id);

    ImGuiID dock_down_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.3f, nullptr, &dock_main_id);
    ImGuiID dock_down_right_id =
        ImGui::DockBuilderSplitNode(dock_down_id, ImGuiDir_Right, 0.6f, nullptr, &dock_down_id);

    ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
    ImGui::DockBuilderDockWindow(HIERARCHY_VIEW, dock_left_id);
    ImGui::DockBuilderDockWindow(INSPECTOR_VIEW, dock_right_id);
    ImGui::DockBuilderDockWindow(STATISTICS_VIEW, dock_right_down_id);

    ImGui::DockBuilderDockWindow(CONSOLE_VIEW, dock_down_id);

    ImGui::DockBuilderDockWindow(CONTENT_VIEW, dock_down_id);
    ImGui::DockBuilderDockWindow(SCENE_VIEW, dock_main_id);
    ImGui::DockBuilderDockWindow(GAME_VIEW, dock_main_id);

    ImGui::DockBuilderFinish(dockspace_id);
}
}


void dockspace::on_frame_ui_render(float headerSize, float footerSize)
{
    const ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags         windowFlags     = 0;
    windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    const ImGuiViewport* viewport      = ImGui::GetMainViewport();
    const ImVec2         dockspaceSize = ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - headerSize - footerSize);
    const ImVec2         dockspacePos  = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + headerSize);
    ImGui::SetNextWindowPos(dockspacePos);
    ImGui::SetNextWindowSize(dockspaceSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Ace Engine Dock Space", nullptr, windowFlags);

    auto dockspace_id = ImGui::GetID("Ace Dockspace");

    if(!ImGui::DockBuilderGetNode(dockspace_id))
    {
        build_dockspace(dockspace_id);
    }
    ImGui::DockSpace(dockspace_id, dockspaceSize, dockspace_flags);
    ImGui::End();

    ImGui::PopStyleVar();

}

void dockspace::execute_dock_builder_order_and_focus_workaround()
{
    // WARNING: BAD HABITS AHEAD!!!

    // only execute if we are in the second frame of our program
    static int i = -1;

    static const std::vector<const char*> focused_dock_tabs{SCENE_VIEW, CONTENT_VIEW};

    int tabs_count = int(focused_dock_tabs.size());
    if(i < tabs_count)
    {
        for(int tab_idx = 0; tab_idx < tabs_count; ++tab_idx)
        {
            if(i == tab_idx)
            {
                ImGui::FocusWindow(ImGui::FindWindowByName(focused_dock_tabs[tab_idx]));
            }
        }

        i++;
    }
}

} // namespace ace
