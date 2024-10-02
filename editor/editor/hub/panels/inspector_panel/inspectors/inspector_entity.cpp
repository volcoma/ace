#include "inspector_entity.h"
#include "inspectors.h"

#include <engine/meta/ecs/components/all_components.h>

#include <editor/imgui/imgui_interface.h>
#include <hpp/type_name.hpp>
#include <hpp/utility.hpp>

namespace ace
{
auto inspector_entity::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata) -> inspect_result
{
    inspect_result result{};
    auto data = var.get_value<entt::handle>();
    if(!data)
        return result;

    hpp::for_each_tuple_type<ace::all_inspectable_components>(
        [&](auto index)
        {
            using ctype = std::tuple_element_t<decltype(index)::value, ace::all_inspectable_components>;
            auto component = data.try_get<ctype>();

            if(!component)
            {
                return;
            }

            bool opened = true;
            auto name = rttr::get_pretty_name(rttr::type::get<ctype>());

            ImGui::PushID(component);
            ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);

            auto pos = ImGui::GetCursorPos();
            auto col_header = ImGui::GetColorU32(ImGuiCol_Header);
            auto col_header_hovered = ImGui::GetColorU32(ImGuiCol_HeaderHovered);
            auto col_header_active = ImGui::GetColorU32(ImGuiCol_HeaderActive);

            auto col_framebg = ImGui::GetColorU32(ImGuiCol_FrameBg);
            auto col_framebg_hovered = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
            auto col_framebg_active = ImGui::GetColorU32(ImGuiCol_FrameBgActive);

            ImGui::PushStyleColor(ImGuiCol_Header, col_framebg);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, col_framebg_hovered);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, col_framebg_active);

            auto popup_str = "COMPONENT_SETTING";
            bool open =
                ImGui::CollapsingHeader(fmt::format("     {}", name).c_str(), nullptr, ImGuiTreeNodeFlags_AllowOverlap);

            bool open_popup = false;
            ImGui::OpenPopupOnItemClick(popup_str);
            ImGui::PopStyleColor(3);

            ImGui::SetCursorPos(pos);
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImColor(col_header), "       %s", ICON_MDI_GRID);

            ImGui::SameLine();
            auto settingsSize = ImGui::CalcTextSize(ICON_MDI_COG).x + ImGui::GetStyle().FramePadding.x * 2.0f;

            auto avail = ImGui::GetContentRegionAvail().x + ImGui::GetStyle().FramePadding.x;
            ImGui::AlignedItem(1.0f,
                               avail,
                               settingsSize,
                               [&]()
                               {
                                   if(ImGui::Button(ICON_MDI_COG))
                                   {
                                       open_popup = true;
                                   }
                               });

            if(open)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
                ImGui::TreePush(name.c_str());

                result |= ::ace::inspect(ctx, component);

                ImGui::TreePop();
                ImGui::PopStyleVar();
            }
            if(open_popup)
                ImGui::OpenPopup(popup_str);

            bool is_popup_open = ImGui::IsPopupOpen(popup_str);
            if(is_popup_open && ImGui::BeginPopupContextWindowEx(popup_str))
            {
                if(ImGui::MenuItem("Reset"))
                {
                    data.remove<ctype>();
                    data.emplace<ctype>();
                }
                bool can_remove = !std::is_same<ctype, id_component>::value &&
                                  !std::is_same<ctype, tag_component>::value &&
                                  !std::is_same<ctype, transform_component>::value;

                ImGui::Separator();
                if(ImGui::MenuItem("Remove Component", nullptr, false, can_remove))
                {
                    data.remove<ctype>();
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();
            if(!opened)
            {
                data.remove<ctype>();
            }
        });

    ImGui::Separator();
    ImGui::NextLine();
    static const auto label = "Add Component";
    auto avail = ImGui::GetContentRegionAvail();
    ImVec2 size = ImGui::CalcItemSize(label);
    size.x *= 2.0f;
    ImGui::AlignedItem(0.5f,
                       avail.x,
                       size.x,
                       [&]()
                       {
                           auto pos = ImGui::GetCursorScreenPos();
                           if(ImGui::Button(label, size))
                           {
                               ImGui::OpenPopup("COMPONENT_MENU");
                               ImGui::SetNextWindowPos(pos);
                           }
                       });

    if(ImGui::BeginPopup("COMPONENT_MENU"))
    {
        ImGui::DrawFilterWithHint(filter_, ICON_MDI_SELECT_SEARCH" Search...", size.x);
        ImGui::DrawItemActivityOutline();

        ImGui::Separator();
        ImGui::BeginChild("COMPONENT_MENU_CONTEXT", ImVec2(ImGui::GetContentRegionAvail().x, size.x));

        hpp::for_each_tuple_type<ace::all_inspectable_components>(
            [&](auto index)
            {
                using ctype = std::tuple_element_t<decltype(index)::value, ace::all_inspectable_components>;

                auto name = rttr::get_pretty_name(rttr::type::get<ctype>());

                if(!filter_.PassFilter(name.c_str()))
                    return;

                if(ImGui::Selectable(name.c_str()))
                {
                    data.remove<ctype>();
                    data.emplace<ctype>();

                    ImGui::CloseCurrentPopup();
                }
            });

        ImGui::EndChild();
        ImGui::EndPopup();
    }

    if(result.changed)
    {
        var = data;
        return result;
    }

    return {};
}
} // namespace ace
