#include "scene_panel.h"
#include "../panels_defs.h"

#include <editor/editing/editing_manager.h>
#include <editor/editing/picking_manager.h>
#include <editor/editing/thumbnail_manager.h>
#include <editor/hub/panels/inspector_panel/inspectors/inspectors.h>

#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/ecs/systems/rendering_path.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/rendering/renderer.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui_widgets/gizmo.h>

#include <filesystem/filesystem.h>
#include <logging/logging.h>
#include <algorithm>
#include <numeric>
namespace ace
{
namespace
{

ImGuiKey delete_key = ImGuiKey_Delete;
ImGuiKey focus_key = ImGuiKey_F;
ImGuiKeyCombination duplicate_combination{ImGuiKey_LeftCtrl, ImGuiKey_D};

void handle_camera_movement(entt::handle camera)
{
    if(!ImGui::IsWindowFocused())
    {
        return;
    }

    if(!ImGui::IsWindowHovered())
    {
        return;
    }

    auto& editor_camera = camera;

    auto& transform = editor_camera.get<transform_component>();
    float movement_speed = 5.0f;
    float rotation_speed = 0.2f;
    float multiplier = 5.0f;

    auto dt = ImGui::GetIO().DeltaTime;
    auto delta_move = ImGui::GetIO().MouseDelta;
    auto delta_wheel = ImGui::GetIO().MouseWheel;

    if(ImGui::IsMouseDown(ImGuiMouseButton_Middle))
    {
        if(ImGui::IsKeyDown(ImGuiKey_LeftShift))
        {
            movement_speed *= multiplier;
        }

        if(delta_move.x != 0)
        {
            transform.move_by_local({-1 * delta_move.x * movement_speed * dt, 0.0f, 0.0f});
        }
        if(delta_move.y != 0)
        {
            transform.move_by_local({0.0f, delta_move.y * movement_speed * dt, 0.0f});
        }
    }

    if(ImGui::IsMouseDown(ImGuiMouseButton_Right))
    {
        if(ImGui::IsKeyDown(ImGuiKey_LeftShift))
        {
            movement_speed *= multiplier;
        }

        if(ImGui::IsKeyDown(ImGuiKey_W))
        {
            transform.move_by_local({0.0f, 0.0f, movement_speed * dt});
        }

        if(ImGui::IsKeyDown(ImGuiKey_S))
        {
            transform.move_by_local({0.0f, 0.0f, -movement_speed * dt});
        }

        if(ImGui::IsKeyDown(ImGuiKey_A))
        {
            transform.move_by_local({-movement_speed * dt, 0.0f, 0.0f});
        }

        if(ImGui::IsKeyDown(ImGuiKey_D))
        {
            transform.move_by_local({movement_speed * dt, 0.0f, 0.0f});
        }
        if(ImGui::IsKeyDown(ImGuiKey_UpArrow))
        {
            transform.move_by_local({0.0f, 0.0f, movement_speed * dt});
        }

        if(ImGui::IsKeyDown(ImGuiKey_DownArrow))
        {
            transform.move_by_local({0.0f, 0.0f, -movement_speed * dt});
        }

        if(ImGui::IsKeyDown(ImGuiKey_LeftArrow))
        {
            transform.move_by_local({-movement_speed * dt, 0.0f, 0.0f});
        }

        if(ImGui::IsKeyDown(ImGuiKey_RightArrow))
        {
            transform.move_by_local({movement_speed * dt, 0.0f, 0.0f});
        }

        auto x = static_cast<float>(delta_move.x);
        auto y = static_cast<float>(delta_move.y);

        if(x != 0.0f || y != 0.0f)
        {
            // Make each pixel correspond to a quarter of a degree.
            float dx = x * rotation_speed;
            float dy = y * rotation_speed;

            transform.rotate_by_euler_global({0.0f, dx, 0.0f});
            transform.rotate_by_euler_local({dy, 0.0f, 0.0f});
        }

        transform.move_by_local({0.0f, 0.0f, 14.0f * movement_speed * delta_wheel * dt});
    }
}

void manipulation_gizmos(entt::handle editor_camera, editing_manager& em)
{
    auto& selected = em.selection_data.object;
    auto& operation = em.operation;
    auto& mode = em.mode;

    auto& camera_trans = editor_camera.get<transform_component>();

    auto& camera_comp = editor_camera.get<camera_component>();
    const auto& camera = camera_comp.get_camera();

    auto p = ImGui::GetItemRectMin();
    auto s = ImGui::GetItemRectSize();
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(p.x, p.y, s.x, s.y);
    ImGuizmo::SetOrthographic(camera.get_projection_mode() == projection_mode::orthographic);

    auto view = camera.get_view().get_matrix();
    static const ImVec2 view_gizmo_sz(100.0f, 100.0f);
    ImGuizmo::ViewManipulate(value_ptr(view),
                             1.0f,
                             p + ImVec2(s.x - view_gizmo_sz.x, 0.0f),
                             view_gizmo_sz,
                             ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 0.0f)));
    math::transform tr = glm::inverse(view);
    camera_trans.set_rotation_local(tr.get_rotation());

    if(!ImGui::IsMouseDown(ImGuiMouseButton_Right) && !ImGui::IsAnyItemActive() && !ImGuizmo::IsUsing())
    {
        if(ImGui::IsKeyPressed(ImGuiKey_Q))
        {
            operation = ImGuizmo::OPERATION::UNIVERSAL;
        }
        if(ImGui::IsKeyPressed(ImGuiKey_W))
        {
            operation = ImGuizmo::OPERATION::TRANSLATE;
        }
        if(ImGui::IsKeyPressed(ImGuiKey_E))
        {
            operation = ImGuizmo::OPERATION::ROTATE;
        }
        if(ImGui::IsKeyPressed(ImGuiKey_R))
        {
            operation = ImGuizmo::OPERATION::SCALE;
        }
        if(ImGui::IsKeyPressed(ImGuiKey_T))
        {
            operation = ImGuizmo::OPERATION::BOUNDS;
        }
    }

    if(selected && selected.is_type<entt::handle>())
    {
        auto sel = selected.get_value<entt::handle>();
        if(sel && sel != editor_camera && sel.all_of<transform_component>())
        {
            auto& transform_comp = sel.get<transform_component>();
            float* snap = nullptr;
            if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
            {
                if(operation == ImGuizmo::OPERATION::TRANSLATE)
                {
                    snap = &em.snap_data.translation_snap[0];
                }
                else if(operation == ImGuizmo::OPERATION::ROTATE)
                {
                    snap = &em.snap_data.rotation_degree_snap;
                }
                else if(operation == ImGuizmo::OPERATION::SCALE)
                {
                    snap = &em.snap_data.scale_snap;
                }
            }

            math::mat4 output = transform_comp.get_transform_global();

            math::mat4 output_delta;
            int movetype = ImGuizmo::Manipulate(camera.get_view(),
                                                camera.get_projection(),
                                                operation,
                                                mode,
                                                math::value_ptr(output),
                                                math::value_ptr(output_delta),
                                                snap);
            if(movetype != ImGuizmo::MT_NONE)
            {
                math::transform delta = output_delta;

                auto perspective = transform_comp.get_perspective_local();
                auto skew = transform_comp.get_skew_local();
                if(ImGuizmo::IsScaleType(movetype))
                {
                    transform_comp.scale_by_local(delta.get_scale());
                }

                if(ImGuizmo::IsRotateType(movetype))
                {
                    transform_comp.rotate_by_global(delta.get_rotation());
                }

                if(ImGuizmo::IsTranslateType(movetype))
                {
                    transform_comp.move_by_global(delta.get_translation());
                }
                transform_comp.set_skew_local(skew);
                transform_comp.set_perspective_local(perspective);
            }

            // if(sel.all_of<model_component>())
            // {
            //     const auto& model_comp = sel.get<model_component>();
            //     const auto& model = model_comp.get_model();
            //     if(!model.is_valid())
            //         return;

            //     const auto lod = model.get_lod(0);
            //     if(!lod)
            //         return;

            //     const auto& mesh = lod.get();
            //     auto rect = mesh.calculate_screen_rect(transform_comp.get_transform_global(), camera);

            //     ImGui::GetWindowDrawList()->AddRect(ImVec2(rect.left, rect.top),
            //                                         ImVec2(rect.right, rect.bottom),
            //                                         ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
            // }
        }
    }
}

static void process_drag_drop_target(rtti::context& ctx, const camera_component& camera_comp)
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

        for(const auto& type : ex::get_suported_formats<mesh>())
        {
            auto payload = ImGui::AcceptDragDropPayload(type.c_str());
            if(payload != nullptr)
            {
                auto cursor_pos = ImGui::GetMousePos();

                std::string absolute_path(reinterpret_cast<const char*>(payload->Data), std::size_t(payload->DataSize));

                std::string key = fs::convert_to_protocol(fs::path(absolute_path)).generic_string();

                auto& def = ctx.get<defaults>();
                auto& es = ctx.get<editing_manager>();
                auto& ec = ctx.get<ecs>();

                auto object = def.create_mesh_entity_at(ctx,
                                                        ec.get_scene(),
                                                        key,
                                                        camera_comp.get_camera(),
                                                        math::vec2{cursor_pos.x, cursor_pos.y});

                es.select(object);
            }
        }

        for(const auto& type : ex::get_suported_formats<prefab>())
        {
            auto payload = ImGui::AcceptDragDropPayload(type.c_str());
            if(payload != nullptr)
            {
                auto cursor_pos = ImGui::GetMousePos();

                std::string absolute_path(reinterpret_cast<const char*>(payload->Data), std::size_t(payload->DataSize));

                std::string key = fs::convert_to_protocol(fs::path(absolute_path)).generic_string();

                auto& def = ctx.get<defaults>();
                auto& es = ctx.get<editing_manager>();
                auto& ec = ctx.get<ecs>();

                auto object = def.create_prefab_at(ctx,
                                                   ec.get_scene(),
                                                   key,
                                                   camera_comp.get_camera(),
                                                   math::vec2{cursor_pos.x, cursor_pos.y});

                es.select(object);
            }
        }

        ImGui::EndDragDropTarget();
    }
}

} // namespace

void scene_panel::draw_menubar(rtti::context& ctx)
{
    auto& em = ctx.get<editing_manager>();
    auto& ec = ctx.get<ecs>();

    if(ImGui::BeginMenuBar())
    {
        float width = ImGui::GetContentRegionAvail().x;
        if(ImGui::MenuItem(ICON_MDI_CURSOR_MOVE, nullptr, em.operation == ImGuizmo::OPERATION::TRANSLATE))
        {
            em.operation = ImGuizmo::OPERATION::TRANSLATE;
        }
        ImGui::SetItemTooltip("%s", "Translate Tool");

        if(ImGui::MenuItem(ICON_MDI_ROTATE_3D_VARIANT, nullptr, em.operation == ImGuizmo::OPERATION::ROTATE))
        {
            em.operation = ImGuizmo::OPERATION::ROTATE;
        }
        ImGui::SetItemTooltip("%s", "Rotate Tool");

        if(ImGui::MenuItem(ICON_MDI_RELATIVE_SCALE, nullptr, em.operation == ImGuizmo::OPERATION::SCALE))
        {
            em.operation = ImGuizmo::OPERATION::SCALE;
            em.mode = ImGuizmo::MODE::LOCAL;
        }
        ImGui::SetItemTooltip("%s", "Scale Tool");

        if(ImGui::MenuItem(ICON_MDI_MOVE_RESIZE, nullptr, em.operation == ImGuizmo::OPERATION::UNIVERSAL))
        {
            em.operation = ImGuizmo::OPERATION::UNIVERSAL;
            em.mode = ImGuizmo::MODE::LOCAL;
        }
        ImGui::SetItemTooltip("%s", "Transform Tool");

        auto icon = em.mode == ImGuizmo::MODE::LOCAL ? ICON_MDI_CUBE "Local" ICON_MDI_ARROW_DOWN_BOLD
                                                     : ICON_MDI_WEB "Global" ICON_MDI_ARROW_DOWN_BOLD;

        if(ImGui::BeginMenu(icon))
        {

            if(ImGui::MenuItem(ICON_MDI_CUBE "Local", nullptr, em.mode == ImGuizmo::MODE::LOCAL))
            {
                em.mode = ImGuizmo::MODE::LOCAL;
            }
            ImGui::SetItemTooltip("%s", "Local Coordinate System");

            if(ImGui::MenuItem(ICON_MDI_WEB "Global", nullptr, em.mode == ImGuizmo::MODE::WORLD))
            {
                em.mode = ImGuizmo::MODE::WORLD;
            }
            ImGui::SetItemTooltip("%s", "Global Coordinate System");

            ImGui::EndMenu();
        }
        ImGui::SetItemTooltip("%s", "Tool's Coordinate System");

        if(ImGui::MenuItem(ICON_MDI_GRID, nullptr, em.show_grid))
        {
            em.show_grid = !em.show_grid;
        }
        ImGui::SetItemTooltip("%s", "Show/Hide Grid");

        if(ImGui::BeginMenu(ICON_MDI_ARROW_DOWN_BOLD, em.show_grid))
        {
            ImGui::PushItemWidth(100.0f);

            ImGui::TextUnformatted("Grid Visual");
            ImGui::LabelText("Grid Plane", "%s", "X Z");
            ImGui::SliderFloat("Grid Opacity", &em.grid_data.opacity, 0.0f, 1.0f);
            ImGui::PopItemWidth();

            ImGui::EndMenu();
        }
        ImGui::SetItemTooltip("%s", "Grid Properties");

        if(ImGui::MenuItem(ICON_MDI_DRAW, nullptr, visualize_passes_))
        {
            visualize_passes_ = !visualize_passes_;
        }
        ImGui::SetItemTooltip("%s", "Visualize Render Passes");

        if(ImGui::BeginMenu(ICON_MDI_GRID_LARGE ICON_MDI_ARROW_DOWN_BOLD))
        {
            ImGui::PushItemWidth(200.0f);
            ImGui::DragVecN("Translation Snap",
                            ImGuiDataType_Float,
                            math::value_ptr(em.snap_data.translation_snap),
                            em.snap_data.translation_snap.length(),
                            0.5f,
                            nullptr,
                            nullptr,
                            "%.2f");

            ImGui::DragFloat("Rotation Degree Snap", &em.snap_data.rotation_degree_snap);
            ImGui::DragFloat("Scale Snap", &em.snap_data.scale_snap);
            ImGui::PopItemWidth();
            ImGui::EndMenu();
        }
        ImGui::SetItemTooltip("%s", "Snapping Properties");

        ImGui::SetNextWindowSizeConstraints({}, {300.0f, ImGui::GetContentRegionAvail().x});
        if(ImGui::BeginMenu(ICON_MDI_CAMERA ICON_MDI_ARROW_DOWN_BOLD))
        {
            ImGui::TextUnformatted("Scene Camera");

            auto& camera_comp = get_camera().get<camera_component>();
            inspect(ctx, camera_comp);

            ImGui::EndMenu();
        }
        ImGui::SetItemTooltip("%s", "Settings for the Scene view camera.");

        ImGui::PushFont(ImGui::Font::Mono);
        auto& io = ImGui::GetIO();
        auto fps_size = ImGui::CalcTextSize(fmt::format("{:.1f}", io.Framerate).c_str()).x;
        ImGui::PopFont();

        ImGui::AlignedItem(1.0f,
                           ImGui::GetContentRegionAvail().x,
                           fps_size,
                           [&]()
                           {
                               ImGui::PushFont(ImGui::Font::Mono);
                               ImGui::Text("%.1f", io.Framerate);
                               ImGui::PopFont();
                           });

        ImGui::EndMenuBar();
    }
}

void scene_panel::init(rtti::context& ctx)
{
    ctx.add<gizmo_registry>();

    gizmos_.init(ctx);

    panel_camera_ = panel_scene_.create_entity();

    auto& transf_comp = panel_camera_.get_or_emplace<transform_component>();
    transf_comp.set_position_local({0.0f, 1.0f, -10.0f});

    panel_camera_.emplace<camera_component>();
}

void scene_panel::deinit(rtti::context& ctx)
{
    gizmos_.deinit(ctx);

    ctx.remove<gizmo_registry>();

}

void scene_panel::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& path = ctx.get<rendering_path>();
    path.prepare_scene(panel_scene_, dt);
}

void scene_panel::draw_scene(rtti::context& ctx, delta_t dt)
{
    auto& path = ctx.get<rendering_path>();
    auto& scene = ctx.get<ecs>().get_scene();

    auto& camera_comp = get_camera().get<camera_component>();
    auto& camera = camera_comp.get_camera();
    auto& render_view = camera_comp.get_render_view();

    path.camera_render_full(scene, camera, render_view, dt);
}

void scene_panel::on_frame_render(rtti::context& ctx, delta_t dt)
{
    if(!is_visible_)
    {
        return;
    }
    draw_scene(ctx, dt);

    gizmos_.on_frame_render(ctx, panel_camera_);
}

void scene_panel::on_frame_ui_render(rtti::context& ctx)
{
    if(ImGui::Begin(SCENE_VIEW, nullptr, ImGuiWindowFlags_MenuBar))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));

        set_visible(true);
        draw_ui(ctx);
    }
    else
    {
        set_visible(false);
    }
    ImGui::End();
}

void scene_panel::draw_ui(rtti::context& ctx)
{
    draw_menubar(ctx);

    auto& em = ctx.get<editing_manager>();

    auto& editor_camera = panel_camera_;

    bool has_edit_camera = editor_camera && editor_camera.all_of<transform_component, camera_component>();

    if(!has_edit_camera)
    {
        return;
    }

    auto size = ImGui::GetContentRegionAvail();
    auto pos = ImGui::GetCursorScreenPos();

    auto& camera_comp = editor_camera.get<camera_component>();
    if(size.x > 0 && size.y > 0)
    {
        camera_comp.get_camera().set_viewport_pos(
            {static_cast<std::uint32_t>(pos.x), static_cast<std::uint32_t>(pos.y)});
        camera_comp.set_viewport_size({static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y)});

        const auto& camera = camera_comp.get_camera();
        auto& render_view = camera_comp.get_render_view();
        const auto& viewport_size = camera.get_viewport_size();
        const auto surface = render_view.get_output_fbo(viewport_size);
        auto tex = surface->get_attachment(0).texture;
        ImGui::Image(ImGui::ToId(tex), size);

        bool is_using = ImGuizmo::IsUsing();
        bool is_over = ImGuizmo::IsOver();
        if(ImGui::IsItemClicked(ImGuiMouseButton_Left) && !is_using)
        {
            auto& selected = em.selection_data.object;

            bool is_over_active_gizmo = is_over && selected && selected.is_type<entt::handle>();
            if(!is_over_active_gizmo)
            {
                ImGui::SetWindowFocus();
                auto& pick_manager = ctx.get<picking_manager>();
                auto pos = ImGui::GetMousePos();
                pick_manager.request_pick({pos.x, pos.y}, camera);
            }
        }

        if(ImGui::IsItemClicked(ImGuiMouseButton_Middle) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::SetWindowFocus();
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
        }

        if(ImGui::IsItemReleased(ImGuiMouseButton_Middle) || ImGui::IsItemReleased(ImGuiMouseButton_Right))
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        }

        // if(ImGui::IsItemKeyPressed(delete_key))
        // {
        //     add_action(
        //         [entity]() mutable
        //         {
        //             entity.destroy();
        //         });
        // }

        // if(ImGui::IsItemKeyPressed(focus_key))
        // {
        //     add_action(
        //         [ctx, entity]() mutable
        //         {
        //             focus_entity(ctx, entity);
        //         });
        // }

        // if(ImGui::IsItemCombinationKeyPressed(duplicate_combination))
        // {
        //     add_action(
        //         [ctx, entity]() mutable
        //         {
        //             auto object = ctx.ec.get_scene().clone_entity(entity);
        //             ctx.em.select(object);
        //         });
        // }

        manipulation_gizmos(editor_camera, em);
        handle_camera_movement(editor_camera);

        if(visualize_passes_)
        {
            static auto light_buffer_format = gfx::get_best_format(BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER,
                                                                   gfx::format_search_flags::four_channels |
                                                                       gfx::format_search_flags::requires_alpha |
                                                                       gfx::format_search_flags::half_precision_float);

            {
                auto refl_buffer = render_view.get_texture("RBUFFER",
                                                           viewport_size.width,
                                                           viewport_size.height,
                                                           false,
                                                           1,
                                                           light_buffer_format);
                ImGui::Image(ImGui::ToId(refl_buffer), size);
            }
            {
                auto light_buffer = render_view.get_texture("LBUFFER",
                                                            viewport_size.width,
                                                            viewport_size.height,
                                                            false,
                                                            1,
                                                            light_buffer_format);
                ImGui::Image(ImGui::ToId(light_buffer), size);
            }

            auto g_buffer_fbo = render_view.get_g_buffer_fbo(viewport_size).get();
            for(std::uint32_t i = 0; i < g_buffer_fbo->get_attachment_count(); ++i)
            {
                const auto tex = g_buffer_fbo->get_attachment(i).texture;
                ImGui::Image(ImGui::ToId(tex), size);
            }
        }
    }

    process_drag_drop_target(ctx, camera_comp);
}

} // namespace ace
