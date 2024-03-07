#include "inspector.h"
#include <imgui/imgui_internal.h>

namespace ace
{

namespace
{
std::vector<property_layout*> stack;
void push_layout_to_stack(property_layout* l)
{
    stack.push_back(l);
}

void pop_layout_from_stack(property_layout* l)
{
    stack.pop_back();
}
}
auto property_layout::get_current() -> property_layout*
{
    return stack.back();
}

property_layout::property_layout()
{
    push_layout_to_stack(this);
}

property_layout::property_layout(const rttr::property& prop, bool columns /*= true*/)
{
    push_layout_to_stack(this);

    set_data(prop, columns);

    push_layout();
}

property_layout::property_layout(const std::string& name, bool columns /*= true*/)
{
    push_layout_to_stack(this);

    set_data(name, {}, columns);

    push_layout();
}

property_layout::property_layout(const std::string& name, const std::string& tooltip, bool columns /*= true*/)
{
    push_layout_to_stack(this);

    set_data(name, tooltip, columns);

    push_layout();
}

property_layout::~property_layout()
{
    pop_layout();

    pop_layout_from_stack(this);
}

void property_layout::set_data(const rttr::property& prop, bool columns)
{
    auto name = rttr::get_pretty_name(prop);

    std::string tooltip{};
    auto meta_tooltip = prop.get_metadata("tooltip");
    if(meta_tooltip)
    {
        tooltip = meta_tooltip.get_value<std::string>();
    }

    set_data(name, tooltip, columns);
}

void property_layout::set_data(const std::string& name, const std::string& tooltip, bool columns)
{
    name_ = name;
    tooltip_ = tooltip;
    columns_ = columns;
}


void property_layout::push_layout()
{
    pushed_ = true;

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

auto property_layout::push_tree_layout(ImGuiTreeNodeFlags flags) -> bool
{
    pushed_ = true;

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

    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    ImGui::AlignTextToFramePadding();
    open_ = ImGui::TreeNodeEx(name_.c_str(), flags | ImGuiTreeNodeFlags_AllowOverlap);

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

    return open_;
}

void property_layout::pop_layout()
{
    if(!pushed_)
    {
        return;
    }

    ImGui::PopID();
    ImGui::PopItemWidth();


    if(open_)
    {
        open_ = false;
        ImGui::TreePop();
    }

    if(columns_)
    {
        columns_ = false;
        if(ImGui::TableGetColumnCount() > 1)
        {
            ImGui::EndTable();
        }
    }


    pushed_ = false;
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
