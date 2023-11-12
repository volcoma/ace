#include "inspector_entity.h"
#include "inspectors.h"

#include <engine/ecs/components/id_component.h>
#include <engine/ecs/components/test_component.h>
#include <engine/ecs/components/transform_component.h>

#include <hpp/type_name.hpp>
#include <hpp/utility.hpp>

namespace ace
{
bool inspector_entity::inspect(rtti::context& ctx,
                               rttr::variant& var,
                               const var_info& info,
                               const meta_getter& get_metadata)
{
    auto data = var.get_value<entt::handle>();
    if(!data)
        return false;
    bool changed = false;

    auto components = data.try_get<ace::id_component, ace::transform_component, ace::test_component>();

    hpp::for_each(components,
                  [&](const auto& component)
                  {
                      if(!component)
                      {
                          return;
                      }
                      using ctype = std::decay_t<decltype(*component)>;

                      bool opened = true;
                      auto component_type = rttr::type::get(*component);
                      std::string name = component_type.get_name().data();
                      auto meta_id = component_type.get_metadata("pretty_name");
                      if(meta_id)
                      {
                          name = meta_id.to_string();
                      }
                      ImGui::PushID(component);
                      ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
                      if(ImGui::CollapsingHeader(name.c_str(), &opened))
                      {
                          ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
                          ImGui::TreePush(name.c_str());

                          rttr::variant component_var = component;
                          changed |= inspect_var(ctx, component_var);

                          ImGui::TreePop();
                          ImGui::PopStyleVar();
                      }
                      ImGui::PopID();
                      if(!opened)
                      {
                          data.remove<ctype>();
                      }
                  });

    ImGui::Separator();
    if(ImGui::Button("+COMPONENT"))
    {
        ImGui::OpenPopup("COMPONENT_MENU");
        ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
    }

    if(ImGui::BeginPopup("COMPONENT_MENU"))
    {
        static ImGuiTextFilter filter;
        filter.Draw("Filter", 180);
        ImGui::Separator();
        ImGui::BeginChild("COMPONENT_MENU_CONTEXT", ImVec2(ImGui::GetContentRegionAvail().x, 200.0f));

        hpp::for_each(components,
                      [&](const auto& component)
                      {
                          using ctype = std::decay_t<decltype(*component)>;

                          auto component_type = rttr::type::get<ctype>();

                          std::string name = component_type.get_name().data();
                          auto meta_id = component_type.get_metadata("pretty_name");
                          if(meta_id)
                          {
                              name = meta_id.to_string();
                          }
                          if(!filter.PassFilter(name.c_str()))
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

    if(changed)
    {
        var = data;
        return true;
    }

    return false;
}
} // namespace ace
