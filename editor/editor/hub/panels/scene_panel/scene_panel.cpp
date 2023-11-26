#include "scene_panel.h"
#include "imgui/imgui.h"
#include "imgui_widgets/gizmo.h"
#include <imgui/imgui_internal.h>

#include <editor/editing/editing_manager.h>
#include <editor/editing/picking_manager.h>

#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/transform_component.h>

#include <algorithm>
#include <engine/ecs/ecs.h>
#include <engine/rendering/renderer.h>
#include <numeric>
namespace ace
{

static bool bar(float _width, float _maxWidth, float _height, const ImVec4& _color)
{
    const ImGuiStyle& style = ImGui::GetStyle();

    ImVec4 hoveredColor(_color.x + _color.x * 0.1f,
                        _color.y + _color.y * 0.1f,
                        _color.z + _color.z * 0.1f,
                        _color.w + _color.w * 0.1f);

    ImGui::PushStyleColor(ImGuiCol_Button, _color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, _color);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, style.ItemSpacing.y));

    bool itemHovered = false;

    ImGui::Button("##barbtn", ImVec2(_width, _height));
    itemHovered |= ImGui::IsItemHovered();

    ImGui::SameLine();
    ImGui::InvisibleButton("##barinvis", ImVec2(_maxWidth - _width + 1, _height));
    itemHovered |= ImGui::IsItemHovered();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    return itemHovered;
}

static void resource_bar(const char* _name,
                         const char* _tooltip,
                         uint32_t _num,
                         uint32_t _max,
                         float _maxWidth,
                         float _height)
{
    bool itemHovered = false;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s: %4d / %4d", _name, _num, _max);
    itemHovered |= ImGui::IsItemHovered();
    ImGui::SameLine();

    const float percentage = float(_num) / float(_max);
    static const ImVec4 color(0.5f, 0.5f, 0.5f, 1.0f);

    itemHovered |= bar(std::max(1.0f, percentage * _maxWidth), _maxWidth, _height, color);
    ImGui::SameLine();

    ImGui::Text("%5.2f%%", double(percentage * 100.0f));

    if(itemHovered)
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s %5.2f%%", _tooltip, double(percentage * 100.0f));
        ImGui::EndTooltip();
    }
}

void show_statistics(bool& show_gbuffer, bool& enable_profiler)
{
    auto& io = ImGui::GetIO();

    auto area = ImGui::GetContentRegionAvail();
    auto stat_pos = ImGui::GetCursorScreenPos();
    ImGui::SetNextWindowPos(stat_pos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), area - ImGui::GetStyle().WindowPadding);
    if(ImGui::Begin(ICON_FA_BAR_CHART "\tSTATISTICS##Scene",
                    nullptr,
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
    {
        auto stats = gfx::get_stats();

        ImGui::Text("Frame %.3f [ms] (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        const double to_cpu_ms = 1000.0 / double(stats->cpuTimerFreq);
        const double to_gpu_ms = 1000.0 / double(stats->gpuTimerFreq);

        if(ImGui::CollapsingHeader(ICON_FA_INFO_CIRCLE "\tRender Info"))
        {
            ImGui::PushFont(ImGui::Font::Mono);

            ImGui::Text("Submit CPU %0.3f, GPU %0.3f (L: %d)",
                        double(stats->cpuTimeEnd - stats->cpuTimeBegin) * to_cpu_ms,
                        double(stats->gpuTimeEnd - stats->gpuTimeBegin) * to_gpu_ms,
                        stats->maxGpuLatency);
            if(-std::numeric_limits<std::int64_t>::max() != stats->gpuMemoryUsed)
            {
                char tmp0[64];
                bx::prettify(tmp0, 64, uint64_t(stats->gpuMemoryUsed));
                char tmp1[64];
                bx::prettify(tmp1, 64, uint64_t(stats->gpuMemoryMax));
                ImGui::Text("GPU mem: %s / %s", tmp0, tmp1);
                char tmp2[64];
                bx::prettify(tmp2, 64, uint64_t(stats->rtMemoryUsed));
                ImGui::Text("Render Target mem: %s / %s", tmp2, tmp0);
                char tmp3[64];
                bx::prettify(tmp3, 64, uint64_t(stats->textureMemoryUsed));
                ImGui::Text("Texture mem: %s / %s", tmp3, tmp0);
            }

            ImGui::Separator();

            std::uint32_t ui_draw_calls = ImGui::GetDrawCalls();
            std::uint32_t ui_primitives = io.MetricsRenderIndices / 3;
            std::uint32_t total_primitives =
                std::accumulate(std::begin(stats->numPrims), std::end(stats->numPrims), 0u);
            ImGui::Text("Total Primitives %u", total_primitives);
            ImGui::Text("UI Primitives: %u", ui_primitives);
            ImGui::Text("Scene Primitives: %u", math::abs<std::uint32_t>(total_primitives - ui_primitives));

            ImGui::Text("Total Draw Calls: %u", stats->numDraw);
            ImGui::Text("UI Draw Calls: %u", ui_draw_calls);
            ImGui::Text("Scene Draw Calls: %u", math::abs<std::uint32_t>(stats->numDraw - ui_draw_calls));
            ImGui::PopFont();
        }
        if(ImGui::CollapsingHeader(ICON_FA_PUZZLE_PIECE "\tResources"))
        {
            const auto caps = gfx::get_caps();

            const float itemHeight = ImGui::GetTextLineHeightWithSpacing();
            const float maxWidth = 90.0f;

            ImGui::PushFont(ImGui::Font::Mono);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Res: Num  / Max");
            resource_bar("DIB",
                         "Dynamic index buffers",
                         stats->numDynamicIndexBuffers,
                         caps->limits.maxDynamicIndexBuffers,
                         maxWidth,
                         itemHeight);
            resource_bar("DVB",
                         "Dynamic vertex buffers",
                         stats->numDynamicVertexBuffers,
                         caps->limits.maxDynamicVertexBuffers,
                         maxWidth,
                         itemHeight);
            resource_bar(" FB",
                         "Frame buffers",
                         stats->numFrameBuffers,
                         caps->limits.maxFrameBuffers,
                         maxWidth,
                         itemHeight);
            resource_bar(" IB",
                         "Index buffers",
                         stats->numIndexBuffers,
                         caps->limits.maxIndexBuffers,
                         maxWidth,
                         itemHeight);
            resource_bar(" OQ",
                         "Occlusion queries",
                         stats->numOcclusionQueries,
                         caps->limits.maxOcclusionQueries,
                         maxWidth,
                         itemHeight);
            resource_bar("  P", "Programs", stats->numPrograms, caps->limits.maxPrograms, maxWidth, itemHeight);
            resource_bar("  S", "Shaders", stats->numShaders, caps->limits.maxShaders, maxWidth, itemHeight);
            resource_bar("  T", "Textures", stats->numTextures, caps->limits.maxTextures, maxWidth, itemHeight);
            resource_bar("  U", "Uniforms", stats->numUniforms, caps->limits.maxUniforms, maxWidth, itemHeight);
            resource_bar(" VB",
                         "Vertex buffers",
                         stats->numVertexBuffers,
                         caps->limits.maxVertexBuffers,
                         maxWidth,
                         itemHeight);
            resource_bar(" VD",
                         "Vertex layouts",
                         stats->numVertexLayouts,
                         caps->limits.maxVertexLayouts,
                         maxWidth,
                         itemHeight);
            ImGui::PopFont();
        }

        if(ImGui::CollapsingHeader(ICON_FA_CLOCK_O "\tProfiler"))
        {
            if(ImGui::Checkbox("Enable profiler", &enable_profiler))
            {
                if(enable_profiler)
                {
                    gfx::set_debug(BGFX_DEBUG_PROFILER);
                }
                else
                {
                    gfx::set_debug(BGFX_DEBUG_NONE);
                }
            }

            ImGui::PushFont(ImGui::Font::Mono);

            if(0 == stats->numViews)
            {
                ImGui::Text("Profiler is not enabled.");
            }
            else
            {
                ImVec4 cpuColor(0.5f, 1.0f, 0.5f, 1.0f);
                ImVec4 gpuColor(0.5f, 0.5f, 1.0f, 1.0f);

                const float itemHeight = ImGui::GetTextLineHeightWithSpacing();
                const float itemHeightWithSpacing = ImGui::GetFrameHeightWithSpacing();
                const double toCpuMs = 1000.0 / double(stats->cpuTimerFreq);
                const double toGpuMs = 1000.0 / double(stats->gpuTimerFreq);
                const float scale = 3.0f;

                if(ImGui::BeginListBox("Encoders",
                                       ImVec2(ImGui::GetWindowWidth(), stats->numEncoders * itemHeightWithSpacing)))
                {
                    ImGuiListClipper clipper;
                    clipper.Begin(stats->numEncoders, itemHeight);

                    while(clipper.Step())
                    {
                        for(int32_t pos = clipper.DisplayStart; pos < clipper.DisplayEnd; ++pos)
                        {
                            const bgfx::EncoderStats& encoderStats = stats->encoderStats[pos];

                            ImGui::Text("%3d", pos);
                            ImGui::SameLine(64.0f);

                            const float maxWidth = 30.0f * scale;
                            const float cpuMs = float((encoderStats.cpuTimeEnd - encoderStats.cpuTimeBegin) * toCpuMs);
                            const float cpuWidth = bx::clamp(cpuMs * scale, 1.0f, maxWidth);

                            if(bar(cpuWidth, maxWidth, itemHeight, cpuColor))
                            {
                                ImGui::SetTooltip("Encoder %d, CPU: %f [ms]", pos, cpuMs);
                            }
                        }
                    }

                    ImGui::EndListBox();
                }

                ImGui::Separator();

                if(ImGui::BeginListBox("Views",
                                       ImVec2(ImGui::GetWindowWidth(), stats->numViews * itemHeightWithSpacing)))
                {
                    ImGuiListClipper clipper;
                    clipper.Begin(stats->numViews, itemHeight);

                    while(clipper.Step())
                    {
                        for(int32_t pos = clipper.DisplayStart; pos < clipper.DisplayEnd; ++pos)
                        {
                            const bgfx::ViewStats& viewStats = stats->viewStats[pos];

                            ImGui::Text("%3d %3d %s", pos, viewStats.view, viewStats.name);

                            const float maxWidth = 30.0f * scale;
                            const float cpuTimeElapsed =
                                float((viewStats.cpuTimeEnd - viewStats.cpuTimeBegin) * toCpuMs);
                            const float gpuTimeElapsed =
                                float((viewStats.gpuTimeEnd - viewStats.gpuTimeBegin) * toGpuMs);
                            const float cpuWidth = bx::clamp(cpuTimeElapsed * scale, 1.0f, maxWidth);
                            const float gpuWidth = bx::clamp(gpuTimeElapsed * scale, 1.0f, maxWidth);

                            ImGui::SameLine(64.0f);

                            if(bar(cpuWidth, maxWidth, itemHeight, cpuColor))
                            {
                                ImGui::SetTooltip("View %d \"%s\", CPU: %f [ms]", pos, viewStats.name, cpuTimeElapsed);
                            }

                            ImGui::SameLine();
                            if(bar(gpuWidth, maxWidth, itemHeight, gpuColor))
                            {
                                ImGui::SetTooltip("View: %d \"%s\", GPU: %f [ms]", pos, viewStats.name, gpuTimeElapsed);
                            }
                        }
                    }

                    ImGui::EndListBox();
                }
            }
            ImGui::PopFont();
        }

        ImGui::Separator();
        ImGui::Checkbox("SHOW G-BUFFER", &show_gbuffer);
    }
    ImGui::End();
}

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

        if(ImGui::IsKeyDown(ImGuiKey_Space))
        {
            transform.move_by_local({0.0f, movement_speed * dt, 0.0f});
        }

        if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
        {
            transform.move_by_local({0.0f, -movement_speed * dt, 0.0f});
        }

        auto x = static_cast<float>(delta_move.x);
        auto y = static_cast<float>(delta_move.y);

         if(x != 0.0f|| y != 0.0f)
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

    auto& camera_comp = editor_camera.get<camera_component>();
    const auto& camera = camera_comp.get_camera();

    auto p = ImGui::GetItemRectMin();
    auto s = ImGui::GetItemRectSize();
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(p.x, p.y, s.x, s.y);
    ImGuizmo::SetOrthographic(camera.get_projection_mode() == projection_mode::orthographic);
    math::mat4 grid(1.0f);
    ImGuizmo::DrawGrid(camera.get_view(), camera.get_projection(), math::value_ptr(grid), 100.0f);


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
            mode = ImGuizmo::MODE::LOCAL;
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
            auto pos = transform_comp.get_position_local();
            auto scale = transform_comp.get_scale_local();
            auto rot = transform_comp.get_rotation_euler_local();

            math::mat4 output;
            ImGuizmo::RecomposeMatrixFromComponents(math::value_ptr(pos), math::value_ptr(rot), math::value_ptr(scale), math::value_ptr(output));

            if(ImGuizmo::Manipulate(camera.get_view(), camera.get_projection(), operation, mode,
								 math::value_ptr(output), nullptr, snap))
            {

                ImGuizmo::DecomposeMatrixToComponents(math::value_ptr(output), math::value_ptr(pos), math::value_ptr(rot), math::value_ptr(scale));

                transform_comp.set_position_local(pos);
                transform_comp.set_scale_local(scale);
                transform_comp.set_rotation_euler_local(rot);

            }

//						if(sel.has_component<model_component>())
//						{
//							const auto model_comp = sel.get_component<model_component>();
//							const auto model_comp_ptr = model_comp.lock().get();
//							const auto& model = model_comp_ptr->get_model();
//							if(!model.is_valid())
//								return;

//							const auto mesh = model.get_lod(0);
//							if(!mesh)
//								return;

//							auto rect = mesh->calculate_screen_rect(transform, camera);

//							gui::GetCurrentWindow()->DrawList->AddRect(ImVec2(rect.left, rect.top),
//																	   ImVec2(rect.right, rect.bottom),
//																	   gui::GetColorU32(ImVec4(1.0f, 0.0f,
//			 0.0f, 1.0f)));
//						}
		}
	}
}

void scene_panel::init(rtti::context& ctx)
{
}

void scene_panel::draw(rtti::context& ctx)
{
    auto& em = ctx.get<editing_manager>();
    auto& ec = ctx.get<ecs>();

    auto& rend = ctx.get<renderer>();

    auto window = rend.get_focused_window();
    auto& editor_camera = ec.editor_camera;
    auto& selected = em.selection_data.object;

    bool has_edit_camera = editor_camera && editor_camera.all_of<transform_component, camera_component>();

    show_statistics(show_gbuffer_, enable_profiler_);

    if(!has_edit_camera)
    {
        return;
    }

    auto size = ImGui::GetContentRegionAvail();
    auto pos = ImGui::GetCursorScreenPos();
    //    draw_selected_camera(size);

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

        if(ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver())
        {
            ImGui::SetWindowFocus();
            auto& pick_manager = ctx.get<picking_manager>();
            auto pos = ImGui::GetMousePos();
            pick_manager.request_pick({pos.x, pos.y});
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

        manipulation_gizmos(editor_camera, em);
        handle_camera_movement(editor_camera);

        //        if(ImGui::IsWindowFocused())
        //        {
        //            ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_Button]);
        //            ImGui::RenderFrameEx(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true, 0.0f, 2.0f);
        //            ImGui::PopStyleColor();

        //            if(input.is_key_pressed(mml::keyboard::Delete))
        //            {
        //                if(selected && selected.is_type<runtime::entity>())
        //                {
        //                    auto sel = selected.get_value<runtime::entity>();
        //                    if(sel && sel != editor_camera)
        //                    {
        //                        sel.destroy();
        //                        es.unselect();
        //                    }
        //                }
        //            }

        //            if(input.is_key_pressed(mml::keyboard::D))
        //            {
        //                if(input.is_key_down(mml::keyboard::LControl))
        //                {
        //                    if(selected && selected.is_type<runtime::entity>())
        //                    {
        //                        auto sel = selected.get_value<runtime::entity>();
        //                        if(sel && sel != editor_camera)
        //                        {
        //                            auto clone = ecs::utils::clone_entity(sel);
        //                            clone.get_component<transform_component>().lock()->set_parent(
        //                                sel.get_component<transform_component>().lock()->get_parent(), false, true);
        //                            es.select(clone);
        //                        }
        //                    }
        //                }
        //            }
        //        }

        if(show_gbuffer_)
        {
            auto g_buffer_fbo = render_view.get_g_buffer_fbo(viewport_size).get();
            for(std::uint32_t i = 0; i < g_buffer_fbo->get_attachment_count(); ++i)
            {
                const auto tex = g_buffer_fbo->get_attachment(i).texture;
                ImGui::Image(ImGui::ToId(tex), size);
            }
        }
    }

    //    process_drag_drop_target(camera_comp);
}

} // namespace ace
