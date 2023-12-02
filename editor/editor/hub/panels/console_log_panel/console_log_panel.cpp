#include "console_log_panel.h"
#include <imgui/imgui_internal.h>

#include <filesystem/filesystem.h>
#include <map>

namespace ace
{

void console_log_panel::sink_it_(const details::log_msg& msg)
{
    {
        std::lock_guard<std::recursive_mutex> lock(entries_mutex_);

        auto log_msg = msg;

        log_msg.color_range_start = 0;
        log_msg.color_range_end = 0;
        log_msg.source = {};
        memory_buf_t formatted;
        formatter_->format(log_msg, formatted);
        log_entry entry;
        entry.formatted.resize(formatted.size());
        std::memcpy(entry.formatted.data(), formatted.data(), formatted.size() * sizeof(char));
        entry.source = msg.source;
        entry.level = msg.level;
        entry.color_range_start = msg.color_range_start;
        entry.color_range_end = msg.color_range_end;

        entry.id = current_id_++;
        entries_.emplace_back(std::move(entry));
    }
    has_new_entries_ = true;
}

void console_log_panel::flush_()
{
}

void console_log_panel::clear_log()
{
    {
        std::lock_guard<std::recursive_mutex> lock(entries_mutex_);
        entries_.clear();
        selected_log_ = {};
    }
    has_new_entries_ = false;
}

auto console_log_panel::has_new_entries() const -> bool
{
    return has_new_entries_;
}

void console_log_panel::set_has_new_entries(bool val)
{
    has_new_entries_ = val;
}

void console_log_panel::draw_range(const mem_buf& formatted, size_t start, size_t end)
{
    if(end > start)
    {
        auto size = (end - start);
        ImGui::TextUnformatted(formatted.data() + start, formatted.data() + start + size);
    }
}

void console_log_panel::draw_log(const log_entry& msg)
{
    static const std::array<ImColor, size_t(level::n_levels)> colors{ImColor{255, 255, 255},
                                                                     ImColor{0, 100, 100},
                                                                     ImColor{0, 180, 0},
                                                                     ImColor{255, 255, 0},
                                                                     ImColor{255, 0, 0},
                                                                     ImColor{180, 0, 0},
                                                                     ImColor{255, 255, 255}};

    ImGui::BeginGroup();
    if(msg.color_range_end > msg.color_range_start)
    {
        // before color range
        draw_range(msg.formatted, 0, msg.color_range_start);
        ImGui::SameLine();

        // in color range
        ImVec4 col = colors[size_t(msg.level)];
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        draw_range(msg.formatted, msg.color_range_start, msg.color_range_end);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        draw_range(msg.formatted, msg.color_range_end, msg.formatted.size());
    }
    else // print without colors if color range is invalid (or color is disabled)
    {
        draw_range(msg.formatted, 0, msg.formatted.size());
    }
    ImGui::SameLine();
    ImGui::Dummy({ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight()});
    ImGui::EndGroup();

    if(ImGui::IsItemClicked())
    {
        select_log(msg);
    }
}

void console_log_panel::draw()
{
    auto avail = ImGui::GetContentRegionAvail();
    if(avail.x < 1.0f || avail.y < 1.0f)
    {
        return;
    }

    if(ImGui::BeginMenuBar())
    {
        filter_.Draw("Filter (inc,-exc)", 200);

        ImGui::SameLine();
        if(ImGui::SmallButton("CLEAR"))
        {
            clear_log();
        }
        ImGui::EndMenuBar();
    }

    ImGui::PushFont(ImGui::Font::Mono);

    //    ImGui::Separator();

    avail = ImGui::GetContentRegionAvail();

    ImGui::SetNextWindowSizeConstraints({}, avail);
    // Display every line as a separate entry so we can change their color or add custom widgets. If you only
    // want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
    // NB- if you have thousands of entries this approach may be too inefficient. You can seek and display
    // only the lines that are visible - CalcListClipping() is a helper to compute this information.
    // If your items are of variable size you may want to implement code similar to what CalcListClipping()
    // does. Or split your data into fixed height items to allow random-seeking into your list.
    ImGui::BeginChild("ScrollingRegion", avail * ImVec2(1.0f, 0.9f), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY);
    if(ImGui::BeginPopupContextWindow())
    {
        if(ImGui::Selectable("Clear"))
            clear_log();
        ImGui::EndPopup();
    }
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

    std::lock_guard<std::recursive_mutex> lock(entries_mutex_);
    display_entries_t entries;
    {
        for(const auto& msg : entries_)
        {
            if(!filter_.PassFilter(msg.formatted.data(), msg.formatted.data() + msg.formatted.size()))
                continue;

            entries.emplace_back(msg);
        }
    }

    ImGuiListClipper clipper;
    clipper.Begin(int(entries.size()));
    while(clipper.Step())
    {
        for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            const auto& msg = entries[i];

            if(selected_log_)
            {
                const auto& selected = *selected_log_;
                if(selected.id == msg.id)
                {
                    auto min = ImGui::GetCursorScreenPos();
                    auto max = min + ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeight());
                    ImGui::RenderFrame(min, max, ImColor(80, 80, 0));
                }
            }

            draw_log(msg);
        }
    }

    if(has_new_entries() && ImGui::GetScrollY() > (ImGui::GetScrollMaxY() - 0.01f))
        ImGui::SetScrollHereY();

    set_has_new_entries(false);

    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::BeginChild("DetailsArea");

    draw_details();
    ImGui::EndChild();

    ImGui::PopFont();
}

void console_log_panel::draw_details()
{
    std::lock_guard<std::recursive_mutex> lock(entries_mutex_);

    if(selected_log_)
    {
        const auto& msg = *selected_log_;
        string_view_t str(msg.formatted.data(), msg.formatted.size());
        auto desc =
            fmt::format("{0}{1}() (at [{2}:{3}])({2})", str, msg.source.funcname, msg.source.filename, msg.source.line);

        ImGui::MarkdownConfig config{};
        config.linkCallback = [](const char* link, uint32_t linkLength)
        {
            fs::path p(std::string(link, linkLength));
            fs::show_in_graphical_env(p);
        };
        ImGui::Markdown(desc.data(), int32_t(desc.size()), config);
    }
}

void console_log_panel::select_log(const log_entry& entry)
{
    selected_log_ = entry;
}

} // namespace ace
