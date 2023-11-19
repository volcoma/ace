#include "hierarchy_graph.h"
#include <imgui/imgui_internal.h>

#include <editor/assets/asset_extensions.h>
#include <editor/editing/editing_system.h>

#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/components/id_component.h>

#include <engine/ecs/ecs.h>

namespace ace
{

namespace
{

ImGuiKey edit_key = ImGuiKey_F2;
ImGuiKey delete_key = ImGuiKey_Delete;
ImGuiKeyCombination duplicate_combination{ImGuiKey_LeftShift, ImGuiKey_D};

using action_t = std::function<void()>;
using actions_t = std::vector<action_t>;
auto get_actions() -> actions_t&
{
    static actions_t actions;
    return actions;
}

void add_action(const action_t& action)
{
    get_actions().emplace_back(action);
}
void execute_actions()
{
    auto& actions = get_actions();
    for(const auto& action : actions)
    {
        action();
    }
    actions.clear();
}

struct graph_context
{
    graph_context(rtti::context& ctx) : es(ctx.get<editing_system>()), ec(ctx.get<ecs>())
    {
    }

    editing_system& es;
    ecs& ec;
};

bool prev_edit_label{};
bool edit_label_{};

auto update_editing()
{
    prev_edit_label = edit_label_;
}

auto is_just_started_editing_label() -> bool
{
    return edit_label_ && edit_label_ != prev_edit_label;
}

auto is_editing_label() -> bool
{
    return edit_label_;
}

void start_editing_label(graph_context& ctx, entt::handle entity)
{
    ctx.es.select(entity);
    edit_label_ = true;
}

void stop_editing_label(graph_context& ctx, entt::handle entity)
{
    edit_label_ = false;
}

auto get_entity_name(entt::handle entity) -> const std::string&
{
    auto& id = entity.get_or_emplace<id_component>();
    return id.name;
}

void set_entity_name(entt::handle entity, const std::string& name)
{
    auto& id = entity.get_or_emplace<id_component>();
    id.name = name;
}

bool process_drag_drop_source(graph_context& ctx, entt::handle entity)
{
    if(entity && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::TextUnformatted(get_entity_name(entity).c_str());
        ImGui::SetDragDropPayload("entity", &entity, sizeof(entity));
        ImGui::EndDragDropSource();
        return true;
    }

    return false;
}

void process_drag_drop_target(graph_context& ctx, entt::handle entity)
{
    if(ImGui::BeginDragDropTarget())
    {
        if(ImGui::IsDragDropPayloadBeingAccepted())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        else
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
        }

        {
            auto payload = ImGui::AcceptDragDropPayload("entity");
            if(payload != nullptr)
            {
                entt::handle dropped{};
                std::memcpy(&dropped, payload->Data, size_t(payload->DataSize));
                if(dropped)
                {
                    auto trans_comp = dropped.try_get<transform_component>();
                    if(trans_comp)
                    {
                        trans_comp->set_parent(entity);
                    }
                }
            }
        }


        ImGui::EndDragDropTarget();
    }
}

void check_drag(graph_context& ctx, entt::handle entity)
{
    if(!process_drag_drop_source(ctx, entity))
    {
        process_drag_drop_target(ctx, entity);
    }
}

void check_context_menu(graph_context& ctx, entt::handle entity)
{
    if(entity)
    {
        if(ImGui::BeginPopupContextItem("Entity Context Menu"))
        {
            if(ImGui::MenuItem("Create Empty Parent"))
            {
                add_action([ctx, entity]() mutable
                {
                    auto& obj_trans_comp = entity.get<transform_component>();
                    auto current_parent = obj_trans_comp.get_parent();

                    auto new_entity = ctx.ec.create_entity(current_parent);
                    obj_trans_comp.set_parent(new_entity);

                    start_editing_label(ctx, new_entity);
                });
            }

            if(ImGui::MenuItem("Create Child"))
            {
                add_action([ctx, entity]() mutable
                {
                    auto new_entity = ctx.ec.create_entity(entity);
                    start_editing_label(ctx, new_entity);
                });


            }

            if(ImGui::MenuItem("Rename", ImGui::GetKeyName(edit_key)))
            {
                add_action([ctx, entity]() mutable
                {
                    start_editing_label(ctx, entity);
                });
            }

            if(ImGui::MenuItem("Duplicate", ImGui::GetKeyCombinationName(duplicate_combination).c_str()))
            {
                //            auto object = ecs::utils::clone_entity(entity);

                //            auto obj_trans_comp = object.get_component<transform_component>().lock();
                //            auto ent_trans_comp = entity.get_component<transform_component>().lock();
                //            if(obj_trans_comp && ent_trans_comp)
                //            {
                //                obj_trans_comp->set_parent(ent_trans_comp->get_parent(), false, true);
                //            }

                //            es.select(object);
            }

            if(ImGui::MenuItem("Delete", ImGui::GetKeyName(delete_key)))
            {
                add_action([ctx, entity]() mutable
                {
                    entity.destroy();
                });
            }

            if(ImGui::MenuItem("Focus", "Shift + F"))
            {
            }
            ImGui::EndPopup();
        }
    }
    else
    {
        if(ImGui::BeginPopupContextWindow())
        {
            if(ImGui::MenuItem("Create Empty"))
            {
                add_action([ctx]() mutable
                {
                    auto new_entity = ctx.ec.create_entity();
                    start_editing_label(ctx, new_entity);
                });

            }
            ImGui::EndPopup();
        }
    }

}

void draw_entity(graph_context& ctx, entt::handle entity)
{
    if(!entity)
    {
        return;
    }

    const auto& name = get_entity_name(entity);
    ImGui::PushID(static_cast<int>(entity.entity()));

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_OpenOnArrow;

    if(ctx.es.is_selected(entity))
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    auto& trans_comp = entity.get<transform_component>();
    bool no_children = trans_comp.get_children().empty();;

    if(no_children)
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    auto pos = ImGui::GetCursorScreenPos() + ImVec2(ImGui::GetTextLineHeightWithSpacing(), 0.0f);
    ImGui::AlignTextToFramePadding();

    auto label = name + "##" + std::to_string(static_cast<int>(entity.entity()));
    bool opened = ImGui::TreeNodeEx(label.c_str(), flags);

    if(ImGui::IsItemReleased(ImGuiMouseButton_Left))
    {
        add_action([ctx, entity]() mutable
        {
            stop_editing_label(ctx, entity);
            ctx.es.select(entity);
        });
    }

    if(ctx.es.is_selected(entity))
    {
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
        }

        if(ImGui::IsItemDoubleClicked(ImGuiMouseButton_Left))
        {
            add_action([ctx, entity]() mutable
            {
                start_editing_label(ctx, entity);
            });
        }

        if(ImGui::IsItemKeyPressed(edit_key))
        {
            add_action([ctx, entity]() mutable
            {
                start_editing_label(ctx, entity);
            });
        }

        if(ImGui::IsItemKeyPressed(delete_key))
        {
            add_action([entity]() mutable
            {
                entity.destroy();
            });
        }

    }


    if(!is_editing_label())
    {
        check_drag(ctx, entity);

        check_context_menu(ctx, entity);
    }


    if(ctx.es.is_selected(entity) && is_editing_label())
    {
        if(is_just_started_editing_label())
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

        auto edit_name = name;
        if(ImGui::InputTextWidget("##rename", edit_name, false, ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            set_entity_name(entity, edit_name);
            stop_editing_label(ctx, entity);
        }

        ImGui::PopItemWidth();

        if(ImGui::IsItemDeactivated())
        {
            stop_editing_label(ctx, entity);
        }
    }

    if(opened)
    {
        if(!no_children)
        {
            const auto& children = trans_comp.get_children();
            for(auto& child : children)
            {
                if(child)
                {
                    draw_entity(ctx, child);
                }
            }
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}
} // namespace

void hierarchy_graph::init(rtti::context& ctx)
{
}

void hierarchy_graph::draw(rtti::context& ctx)
{
    update_editing();
    execute_actions();

    graph_context gctx(ctx);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings;

    if(ImGui::BeginChild("hierarchy_content", ImGui::GetContentRegionAvail(), 0, flags))
    {
        check_context_menu(gctx, {});

        if(ImGui::Button("TEST"))
        {
            gctx.ec.create_test_scene();
        }

        gctx.ec.registry.view<transform_component, root_component>().each(
            [&](auto e, auto&& comp, auto&& tag)
            {
                draw_entity(gctx, comp.get_owner());
            });
    }
    ImGui::EndChild();
    check_drag(gctx, {});
}

} // namespace ace
