#include "hierarchy_graph.h"
#include <imgui/imgui_internal.h>

#include <editor/editing/editing_manager.h>

#include <engine/ecs/components/id_component.h>
#include <engine/ecs/components/transform_component.h>

#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/model_component.h>

#include <engine/defaults/defaults.h>
#include <engine/ecs/ecs.h>

#include <engine/rendering/mesh.h>
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
    graph_context(rtti::context& context)
        : ctx(context)
        , def(context.get<defaults>())
        , em(context.get<editing_manager>())
        , ec(context.get<ecs>())
    {
    }

    rtti::context& ctx;
    defaults& def;
    editing_manager& em;
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
    ctx.em.select(entity);
    edit_label_ = true;
}

void stop_editing_label(graph_context& ctx, entt::handle entity)
{
    edit_label_ = false;
}

auto get_entity_tag(entt::handle entity) -> const std::string&
{
    auto& tag = entity.get_or_emplace<tag_component>();
    return tag.tag;
}

void set_entity_tag(entt::handle entity, const std::string& name)
{
    auto& tag = entity.get_or_emplace<tag_component>();
    tag.tag = name;
}

bool process_drag_drop_source(graph_context& ctx, entt::handle entity)
{
    if(entity && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::TextUnformatted(get_entity_tag(entity).c_str());
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

math::bbox calc_bounds(entt::handle entity)
{
    const math::vec3 one = {1.0f, 1.0f, 1.0f};
    math::bbox bounds = math::bbox(-one, one);
    auto& ent_trans_comp = entity.get<transform_component>();
    {
        auto target_pos = ent_trans_comp.get_position_global();
        bounds = math::bbox(target_pos - one, target_pos + one);

        auto ent_model_comp = entity.try_get<model_component>();
        if(ent_model_comp)
        {
            const auto& model = ent_model_comp->get_model();
            if(model.is_valid())
            {
                const auto lod = model.get_lod(0);
                if(lod)
                {
                    const auto& mesh = lod.get();
                    bounds = mesh.get_bounds();
                }
            }
        }
        const auto& world = ent_trans_comp.get_transform_global();
        bounds = math::bbox::mul(bounds, world);
    }
    return bounds;
};

void focus_entity_on_bounds(entt::handle entity, const math::bbox& bounds)
{
    auto& trans_comp = entity.get<transform_component>();
    auto& camera_comp = entity.get<camera_component>();
    const auto& cam = camera_comp.get_camera();

    math::vec3 cen = bounds.get_center();
    math::vec3 size = bounds.get_dimensions();

    float aspect = cam.get_aspect_ratio();
    float fov = cam.get_fov();
    // Get the radius of a sphere circumscribing the bounds
    float radius = math::length(size) / 2.0f;
    // Get the horizontal FOV, since it may be the limiting of the two FOVs to properly
    // encapsulate the objects
    float horizontalFOV = math::degrees(2.0f * math::atan(math::tan(math::radians(fov) / 2.0f) * aspect));
    // Use the smaller FOV as it limits what would get cut off by the frustum
    float mfov = math::min(fov, horizontalFOV);
    float dist = radius / (math::sin(math::radians(mfov) / 2.0f));

    camera_comp.set_ortho_size(radius);
    trans_comp.set_position_global(cen - dist * trans_comp.get_z_axis_global());
    trans_comp.look_at(cen);
}

void check_context_menu(graph_context& ctx, entt::handle entity)
{
    if(entity)
    {
        if(ImGui::BeginPopupContextItem("Entity Context Menu"))
        {
            if(ImGui::MenuItem("Create Empty Parent"))
            {
                add_action(
                    [ctx, entity]() mutable
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
                add_action(
                    [ctx, entity]() mutable
                    {
                        auto new_entity = ctx.ec.create_entity(entity);
                        start_editing_label(ctx, new_entity);
                    });
            }

            if(ImGui::MenuItem("Rename", ImGui::GetKeyName(edit_key)))
            {
                add_action(
                    [ctx, entity]() mutable
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
                add_action(
                    [ctx, entity]() mutable
                    {
                        entity.destroy();
                    });
            }

            if(ImGui::MenuItem("Focus", "Shift + F"))
            {
                auto& editor_camera = ctx.ec.editor_camera;
                if(editor_camera.all_of<transform_component, camera_component>())
                {
                    auto bounds = calc_bounds(entity);
                    focus_entity_on_bounds(editor_camera, bounds);
                }
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
                add_action(
                    [ctx]() mutable
                    {
                        auto new_entity = ctx.ec.create_entity();
                        start_editing_label(ctx, new_entity);
                    });
            }
            if(ImGui::BeginMenu("3D Objects"))
            {
                static const std::map<std::string, std::vector<std::string>> menu_objects = {
                    {"Basic", {"Sphere", "Cube", "Plane", "Cylinder", "Capsule", "Cone", "Torus", "Teapot"}},
                    {"Polygons", {"Icosahedron", "Dodecahedron"}},
                    {"Icospheres", {"Icosphere0",  "Icosphere1",  "Icosphere2",  "Icosphere3",  "Icosphere4",
                                    "Icosphere5",  "Icosphere6",  "Icosphere7",  "Icosphere8",  "Icosphere9",
                                    "Icosphere10", "Icosphere11", "Icosphere12", "Icosphere13", "Icosphere14",
                                    "Icosphere15", "Icosphere16", "Icosphere17", "Icosphere18", "Icosphere19"}}};

                //                auto& am = core::get_subsystem<runtime::asset_manager>();
                for(const auto& p : menu_objects)
                {
                    const auto& name = p.first;
                    const auto& objects_name = p.second;

                    if(ImGui::BeginMenu(name.c_str()))
                    {
                        for(const auto& name : objects_name)
                        {
                            if(ImGui::MenuItem(name.c_str()))
                            {
                                auto object = ctx.def.create_mesh_entity(ctx.ctx, name);
                                ctx.em.select(object);
                            }
                        }
                        ImGui::EndMenu();
                    }
                }
                ImGui::EndMenu();
            }

            if(ImGui::BeginMenu("Lighting"))
            {
                if(ImGui::BeginMenu("Light"))
                {
                    static const std::vector<std::pair<std::string, light_type>> light_objects = {
                        {"Directional", light_type::directional},
                        {"Spot", light_type::spot},
                        {"Point", light_type::point}};

                    for(const auto& p : light_objects)
                    {
                        const auto& name = p.first;
                        const auto& type = p.second;
                        if(ImGui::MenuItem(name.c_str()))
                        {
                            auto object = ctx.def.create_light_entity(ctx.ctx, type, name);
                            ctx.em.select(object);
                        }
                    }
                    ImGui::EndMenu();
                }

                if(ImGui::BeginMenu("Reflection Probes"))
                {
                    static const std::vector<std::pair<std::string, probe_type>> reflection_probes = {
                        {"Sphere", probe_type::sphere},
                        {"Box", probe_type::box}};
                    for(const auto& p : reflection_probes)
                    {
                        const auto& name = p.first;
                        const auto& type = p.second;

                        if(ImGui::MenuItem(name.c_str()))
                        {
                            auto object = ctx.def.create_reflection_probe_entity(ctx.ctx, type, name);
                            ctx.em.select(object);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }

            //            if(ImGui::BeginMenu("AUDIO"))
            //            {
            //                if(ImGui::MenuItem("SOURCE"))
            //                {
            //                    auto object = ecs.create();
            //                    object.set_name("AUDIO SOURCE");
            //                    object.assign<transform_component>();
            //                    object.assign<audio_source_component>();
            //                    es.select(object);
            //                }
            //                ImGui::EndMenu();
            //            }

            if(ImGui::MenuItem("Camera"))
            {
                auto object = ctx.def.create_camera_entity(ctx.ctx, "Camera");
                ctx.em.select(object);
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

    const auto& name = get_entity_tag(entity);
    ImGui::PushID(static_cast<int>(entity.entity()));

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_OpenOnArrow;

    if(ctx.em.is_selected(entity))
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    auto& trans_comp = entity.get<transform_component>();
    bool no_children = trans_comp.get_children().empty();
    ;

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
        add_action(
            [ctx, entity]() mutable
            {
                stop_editing_label(ctx, entity);
                ctx.em.select(entity);
            });
    }

    if(ctx.em.is_selected(entity))
    {
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
        }

        if(ImGui::IsItemDoubleClicked(ImGuiMouseButton_Left))
        {
            add_action(
                [ctx, entity]() mutable
                {
                    start_editing_label(ctx, entity);
                });
        }

        if(ImGui::IsItemKeyPressed(edit_key))
        {
            add_action(
                [ctx, entity]() mutable
                {
                    start_editing_label(ctx, entity);
                });
        }

        if(ImGui::IsItemKeyPressed(delete_key))
        {
            add_action(
                [entity]() mutable
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

    if(ctx.em.is_selected(entity) && is_editing_label())
    {
        if(is_just_started_editing_label())
        {
            ImGui::SetKeyboardFocusHere();
        }

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

        auto edit_name = name;
        if(ImGui::InputTextWidget("##rename",
                                  edit_name,
                                  false,
                                  ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            set_entity_tag(entity, edit_name);
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
