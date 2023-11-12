#include "inspector.h"
#include <imgui/imgui_internal.h>

namespace ace
{
property_layout::property_layout(const rttr::property& prop, bool columns /*= true*/)
{
    columns_ = columns;
    std::string pretty_name = prop.get_name().to_string();
    auto meta_pretty_name = prop.get_metadata("pretty_name");
    if(meta_pretty_name)
    {
        pretty_name = meta_pretty_name.get_value<std::string>();
    }

    auto meta_tooltip = prop.get_metadata("tooltip");
    if(meta_tooltip)
    {
        tooltip_ = meta_tooltip.get_value<std::string>();
    }

    name_ = pretty_name;

    push_layout();
}

property_layout::property_layout(const std::string& name, bool columns /*= true*/)
{
    columns_ = columns;

    push_layout();
}

property_layout::property_layout(const std::string& name, const std::string& tooltip, bool columns /*= true*/)
{
    columns_ = columns;
    name_ = name;

    push_layout();
}

property_layout::~property_layout()
{
    pop_layout();
}

void property_layout::push_layout()
{
    if(columns_)
    {
        auto avail = ImGui::GetContentRegionAvail();

        ImGui::BeginTable(("properties##" + name_).c_str(), 2);

        auto first_column = 0.375f;
        ImGui::TableSetupColumn("##prop_column1", ImGuiTableColumnFlags_WidthFixed, avail.x * first_column);
        ImGui::TableSetupColumn("##prop_column2", ImGuiTableColumnFlags_WidthFixed, avail.x * (1.0f - first_column));

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(name_.c_str());

    if(!tooltip_.empty())
    {
        ImGui::SameLine();
        ImGui::HelpMarker(tooltip_.c_str());
    }

    if(columns_)
    {
        ImGui::TableNextColumn();
    }

    ImGui::PushID(name_.c_str());
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
}

void property_layout::pop_layout()
{
    ImGui::PopID();
    ImGui::PopItemWidth();
    if(columns_)
    {
        if(ImGui::TableGetColumnCount() > 1)
        {
            ImGui::EndTable();
        }
    }
}

void inspector::before_inspect(const rttr::property& prop)
{
    layout_ = std::make_unique<property_layout>(prop);
}

void inspector::after_inspect(const rttr::property& prop)
{
    layout_.reset();
}

}
