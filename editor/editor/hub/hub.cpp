#include "hub.h"
#include "hpp/optional.hpp"
#include <editor/events.h>
#include <editor/system/project_manager.h>
#include <engine/events.h>
#include <engine/rendering/renderer.h>
#include <editor/hub/panels/inspector_panel/inspectors/inspectors.h>

#include <filedialog/filedialog.h>
#include <imgui/imgui.h>
#include <memory>

namespace ace
{

hub::hub(rtti::context& ctx)
{
    auto& ui_ev = ctx.get<ui_events>();
    auto& ev = ctx.get<events>();

    ev.on_frame_update.connect(sentinel_, this, &hub::on_frame_update);
    ev.on_frame_render.connect(sentinel_, this, &hub::on_frame_render);

    ui_ev.on_frame_ui_render.connect(sentinel_, this, &hub::on_frame_ui_render);
}

auto hub::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    panels_.init(ctx);

    return true;
}

auto hub::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    panels_.deinit(ctx);

    return true;
}

void hub::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& pm = ctx.get<project_manager>();

    if(!pm.has_open_project())
    {
        return;
    }
    panels_.on_frame_update(ctx, dt);
}

void hub::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& pm = ctx.get<project_manager>();

    if(!pm.has_open_project())
    {
        return;
    }
    panels_.on_frame_render(ctx, dt);
}

void hub::on_frame_ui_render(rtti::context& ctx, delta_t dt)
{
    auto& pm = ctx.get<project_manager>();

    if(!pm.has_open_project())
    {
        on_start_page_render(ctx);
    }
    else
    {
        on_opened_project_render(ctx);
    }
}

void hub::on_opened_project_render(rtti::context& ctx)
{
    panels_.on_frame_ui_render(ctx);
}

void hub::on_start_page_render(rtti::context& ctx)
{
    auto& pm = ctx.get<project_manager>();

    auto on_create_project = [&](const std::string& p)
    {
        auto path = fs::path(p).make_preferred();
        pm.create_project(ctx, path);
    };
    auto on_open_project = [&](const std::string& p)
    {
        auto path = fs::path(p).make_preferred();
        pm.open_project(ctx, path);
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

    ImGui::OpenPopup("RECENT PROJECTS");
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.5f, ImGuiCond_Appearing);

    if(ImGui::BeginPopupModal("RECENT PROJECTS", nullptr, ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::BeginGroup();
        {
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_HorizontalScrollbar |
                                     ImGuiWindowFlags_NoSavedSettings;

            if(ImGui::BeginChild("projects_content",
                                 ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, ImGui::GetContentRegionAvail().y),
                                 0,
                                 flags))
            {
                const auto& rencent_projects = pm.get_options().recent_projects;
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

                new_project_creator = true;
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



    //     if(new_project_creator)
    //     {
    //         ImGui::OpenPopup("CREATE NEW PROJECT");
    //         ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.5f, ImGuiCond_Appearing);
    //     }

    //     if(ImGui::BeginPopupModal("CREATE NEW PROJECT", nullptr, ImGuiWindowFlags_NoSavedSettings))
    //     {
    //         static std::string name;
    //         static std::string version;
    //         static fs::path location{};

    //         rttr::variant vname(name);

    //         ImGui::Button("SADASASDA");

    //         if(inspect_var(ctx, vname).changed)
    //         {
    //         }

    //         rttr::variant vversion(version);

    //         if(inspect_var(ctx, vversion).changed)
    //         {
    //         }

    //         rttr::variant vlocation(location);

    //         if(inspect_var(ctx, vlocation).changed)
    //         {
    //         }

    //         ImGui::EndPopup();
    //     }
        ImGui::EndPopup();
    }





    ImGui::End();
}

} // namespace ace
