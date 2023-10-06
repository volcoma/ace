#include "hub.h"
#include <editor/events.h>
#include <engine/rendering/renderer.h>

#include "../system/project_manager.h"
#include <filedialog/filedialog.h>

namespace ace
{

hub::hub(rtti::context& ctx)
{
    auto& ev = ctx.get<ui_events>();

    ev.on_frame_ui_render.connect(sentinel_, this, &hub::on_frame_ui_render);
}

void hub::on_frame_ui_render(rtti::context& ctx, delta_t dt)
{
    auto& pm = ctx.get<project_manager>();
    auto& rend = ctx.get<renderer>();

    if(has_open_project_)
    {
        panels_.draw(ctx);
        return;
    }

    auto on_project_opened = [&]()
    {
        const auto& main_window = rend.get_main_window();
        main_window->get_window().maximize();
        rend.show_all_secondary_windows();
        has_open_project_ = true;
    };

    auto on_create_project = [&](const std::string& p)
    {
        auto path = fs::path(p).make_preferred();
        pm.create_project(ctx, path);
        on_project_opened();
    };
    auto on_open_project = [&](const std::string& p)
    {
        auto path = fs::path(p).make_preferred();
        pm.open_project(ctx, path);
        on_project_opened();
    };



    //    ImGui::PushFont("standard_big");
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::Begin("START PAGE", nullptr, window_flags);

    ImGui::PopStyleVar(2);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings;

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("RECENT PROJECTS");
    ImGui::Separator();
    ImGui::BeginGroup();
    {
        if(ImGui::BeginChild("projects_content",
                             ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, ImGui::GetContentRegionAvail().y),
                             false,
                             flags))
        {
            const auto& rencent_projects = pm.get_options().recent_project_paths;
            for(const auto& path : rencent_projects)
            {
                if(ImGui::Selectable(path.c_str()))
                {
                    on_open_project(path);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::BeginGroup();
    {
        if(ImGui::Button("NEW PROJECT", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
        {
            std::string path;
            if(native::pick_folder_dialog(path))
            {
                on_create_project(path);
            }
        }

        if(ImGui::Button("OPEN OTHER", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
        {
            std::string path;
            if(native::pick_folder_dialog(path))
            {
                on_open_project(path);
            }
        }
    }
    ImGui::EndGroup();
    //	ImGui::PopFont();
    ImGui::End();
}

} // namespace ace
