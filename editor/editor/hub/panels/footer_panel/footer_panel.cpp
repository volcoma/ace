#include "footer_panel.h"
#include "../panels_defs.h"

#include <engine/threading/threader.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui_widgets/tooltips.h>
#include <imgui_widgets/utils.h>

#include <logging/logging.h>

namespace ace
{

void draw_footer_child(rtti::context& ctx, float footerSize, const std::function<void()>& on_draw)
{
    ImGuiWindowFlags headerFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoDecoration;
    const std::string childID = "FOOTER_menubar";
    ImGui::BeginChild(childID.c_str(), ImVec2(0, 0), false, headerFlags);
    on_draw();

    ImGui::SameLine();

    auto threads = itc::get_all_registered_threads();
    size_t total_jobs = 0;
    for(const auto& id : threads)
    {
        total_jobs += itc::get_pending_task_count(id);
    }

    auto& thr = ctx.get<threader>();
    auto pool_jobs = thr.pool->get_jobs_count();

    auto jobs_icon = fmt::format("{} {}", total_jobs, ICON_MDI_BUS_ALERT);

    ImGui::AlignedItem(
        1.0f,
        ImGui::GetContentRegionAvail().x,
        ImGui::CalcTextSize(jobs_icon.c_str()).x,
        [&]()
        {
            ImGui::HelpMarker(
                jobs_icon.c_str(),
                false,
                [&]()
                {
                    ImGui::TextUnformatted(
                        fmt::format("Threads : {}, Jobs : {}, Pool Jobs {}", threads.size(), total_jobs, pool_jobs)
                            .c_str());
                    for(const auto& id : threads)
                    {
                        auto jobs_info = itc::get_pending_task_count_detailed(id);
                        ImGui::TextUnformatted(
                            fmt::format("Thread : {}, Jobs : {}", jobs_info.thread_name, jobs_info.count).c_str());
                    }
                });
        });

    ImGui::EndChild();
}

void footer_panel::on_frame_ui_render(rtti::context& ctx, float footerSize, const std::function<void()>& on_draw)
{
    ImGuiWindowFlags footerFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDecoration;
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - footerSize));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, footerSize));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    ImGui::SetNextWindowViewport(viewport->ID);
    if(ImGui::Begin("FOOTER", nullptr, footerFlags))
    {
        // Draw a sep. child for the menu bar.
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_MenuBarBg));
        draw_footer_child(ctx, footerSize, on_draw);

        ImGui::PopStyleColor();
    }

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

} // namespace ace
