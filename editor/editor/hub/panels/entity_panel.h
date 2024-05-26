#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/ecs/ecs.h>

#include <editor/imgui/integration/imgui.h>

namespace ace
{
class entity_panel
{
public:
    ~entity_panel() = default;

    using action_t = std::function<void()>;
    using actions_t = std::vector<action_t>;

    void on_frame_ui_render();

    void duplicate_entity(entt::handle entity);
    void focus_entity(entt::handle camera, entt::handle entity);
    void delete_entity(entt::handle entity);

    void add_action(const action_t& action);


    ImGuiKey edit_key = ImGuiKey_F2;
    ImGuiKey delete_key = ImGuiKey_Delete;
    ImGuiKey focus_key = ImGuiKey_F;
    ImGuiKeyCombination duplicate_combination{ImGuiKey_LeftCtrl, ImGuiKey_D};


protected:
    void execute_actions();

    actions_t actions;


};
} // namespace ace
