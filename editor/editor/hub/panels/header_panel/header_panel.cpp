#include "header_panel.h"
#include "../panels_defs.h"

#include <editor/editing/editing_manager.h>
#include <editor/system/project_manager.h>

#include <engine/defaults/defaults.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/threading/threader.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace ace
{
namespace
{
void draw_menubar_child(rtti::context& ctx)
{
    ImGuiWindowFlags headerFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar;
    const std::string childID = "HEADER_menubar";
    ImGui::BeginChild(childID.c_str(), ImVec2(0, ImGui::GetFrameHeight() - 2), false, headerFlags);

    // Draw menu bar.
    if(ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New Scene", "Ctrl + N"))
            {
                auto& em = ctx.get<editing_manager>();
                em.close_project();

                auto& ec = ctx.get<ecs>();
                ec.unload_scene();

                auto& def = ctx.get<defaults>();
                def.create_default_3d_scene(ctx, ec.get_scene());
            }

            if(ImGui::MenuItem("Open Scene", "Ctrl + O"))
            {
                auto& em = ctx.get<editing_manager>();
                em.close_project();

                auto& ec = ctx.get<ecs>();
                ec.unload_scene();

                auto& def = ctx.get<defaults>();
                def.create_default_3d_scene(ctx, ec.get_scene());
            }

            if(ImGui::MenuItem("Close", nullptr))
            {
                auto& pm = ctx.get<project_manager>();
                pm.close_project(ctx);
            }
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Help"))
        {
            if(ImGui::MenuItem("About"))
            {
            }

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::EndChild();
}

void draw_play_toolbar(rtti::context& ctx)
{
    float width = ImGui::GetContentRegionAvail().x;

    const auto& style = ImGui::GetStyle();
    auto framePadding = style.FramePadding;
    auto itemSpacing = style.ItemSpacing;
    ImGui::AlignedItem(0.5f,
                       width,
                       ImGui::CalcTextSize(ICON_MDI_PLAY ICON_MDI_PAUSE ICON_MDI_SKIP_NEXT).x +
                           style.FramePadding.x * 6 + itemSpacing.x * 2,
                       [&]()
                       {
                           auto& ev = ctx.get<events>();

                           ImGui::BeginGroup();
                           if(ImGui::Button(ev.is_playing ? ICON_MDI_STOP : ICON_MDI_PLAY))
                           {
                               ev.toggle_play_mode(ctx);

                               ImGui::FocusWindow(ImGui::FindWindowByName(ev.is_playing ? GAME_VIEW : SCENE_VIEW));
                           }
                           ImGui::SameLine();
                           if(ImGui::Button(ICON_MDI_PAUSE))
                           {
                               auto& ev = ctx.get<events>();

                               bool was_playing = ev.is_playing;
                               ev.toggle_pause(ctx);
                           }
                           ImGui::SameLine();
                           if(ImGui::Button(ICON_MDI_SKIP_NEXT))
                           {
                           }

                           ImGui::EndGroup();

                           if(ev.is_playing)
                           {
                               ImGui::RenderFocusFrame(ImGui::GetItemRectMin(),
                                                       ImGui::GetItemRectMax(),
                                                       ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 0.0f, 1.0f)));
                           }
                       });
}
} // namespace

void header_panel::draw(rtti::context& ctx, float headerSize)
{
    ImGuiWindowFlags headerFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, headerSize));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    ImGui::SetNextWindowViewport(viewport->ID);

    if(ImGui::Begin("HEADER", nullptr, headerFlags))
    {
        // Draw a sep. child for the menu bar.
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        draw_menubar_child(ctx);
        draw_play_toolbar(ctx);
        ImGui::PopStyleColor();
    }

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

} // namespace ace
