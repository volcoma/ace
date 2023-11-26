#include "inspector_entity.h"
#include "inspectors.h"

#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/id_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
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

    auto components = data.try_get<id_component,
                                   tag_component,
                                   transform_component,
                                   test_component,
                                   model_component,
                                   camera_component,
                                   light_component,
                                   reflection_probe_component>();

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
    ImGui::NextLine();
    static const auto label = "Add Component";
    auto avail = ImGui::GetContentRegionAvail();
    ImVec2 size = ImGui::CalcItemSize(label);
    size.x *= 2.0f;
    ImGui::AlignedItem(0.5f, avail.x, size.x, [&]()
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
        static ImGuiTextFilter filter;
        filter.Draw("##Filter", size.x);
        ImGui::Separator();
        ImGui::BeginChild("COMPONENT_MENU_CONTEXT", ImVec2(ImGui::GetContentRegionAvail().x, size.x));

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
