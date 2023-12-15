#include "scene_panel.h"
#include "imgui/imgui.h"
#include "logging/logging.h"
#include "math/transform.hpp"
#include <imgui/imgui_internal.h>
#include <imgui_widgets/gizmo.h>

#include <editor/editing/editing_manager.h>
#include <editor/editing/picking_manager.h>
#include <editor/editing/thumbnail_manager.h>

#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/rendering/model.h>
#include <engine/rendering/renderer.h>

#include <algorithm>
#include <filesystem/filesystem.h>
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
    if(ImGui::Begin(ICON_MDI_CHART_BAR "\tSTATISTICS##Scene",
                    nullptr,
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
    {
        auto overlayWidth = ImGui::GetContentRegionAvail().x;
        auto stats = gfx::get_stats();

        // update 10 times per second
        static constexpr float GRAPH_FREQUENCY = 0.1f;
        // show 100 values
        static constexpr size_t GRAPH_HISTORY = 100;

        // plots
        static float fpsValues[GRAPH_HISTORY] = {0};
        static float frameTimeValues[GRAPH_HISTORY] = {0};
        static float gpuMemoryValues[GRAPH_HISTORY] = {0};
        static float rtMemoryValues[GRAPH_HISTORY] = {0};
        static float textureMemoryValues[GRAPH_HISTORY] = {0};

        static size_t offset = 0;

        static float mTime = 0.0f;
        mTime += io.DeltaTime;

        // update after drawing so offset is the current value
        static float oldTime = 0.0f;
        if(mTime - oldTime > GRAPH_FREQUENCY)
        {
            offset = (offset + 1) % GRAPH_HISTORY;
            ImGuiIO& io = ImGui::GetIO();
            fpsValues[offset] = 1 / io.DeltaTime;
            frameTimeValues[offset] = io.DeltaTime * 1000;
            gpuMemoryValues[offset] = float(stats->gpuMemoryUsed) / 1024 / 1024;
            rtMemoryValues[offset] = float(stats->rtMemoryUsed) / 1024 / 1024;
            textureMemoryValues[offset] = float(stats->textureMemoryUsed) / 1024 / 1024;

            oldTime = mTime;
        }

        //        ImGui::Text("Frame %.3f [ms] (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        const double to_cpu_ms = 1000.0 / double(stats->cpuTimerFreq);
        const double to_gpu_ms = 1000.0 / double(stats->gpuTimerFreq);

        //        if(app.config->overlays.fps)
        {
            ImGui::Separator();
            ImGui::Text("FPS %.1f", io.Framerate);
            ImGui::PlotLines("",
                             fpsValues,
                             IM_ARRAYSIZE(fpsValues),
                             (int)offset + 1,
                             nullptr,
                             0.0f,
                             200.0f,
                             ImVec2(overlayWidth, 50));
        }
        //        if(app.config->overlays.frameTime)
        {
            ImGui::Separator();
            ImGui::Text("Frame time %.3f ms", 1000.0f / io.Framerate);
            ImGui::PlotLines("",
                             frameTimeValues,
                             IM_ARRAYSIZE(frameTimeValues),
                             (int)offset + 1,
                             nullptr,
                             0.0f,
                             30.0f,
                             ImVec2(overlayWidth, 50));
        }

        if(ImGui::CollapsingHeader(ICON_MDI_INFORMATION "\tRender Info"))
        {
            ImGui::PushFont(ImGui::Font::Mono);

            ImGui::Text("Submit CPU %0.3f, GPU %0.3f (L: %d)",
                        double(stats->cpuTimeEnd - stats->cpuTimeBegin) * to_cpu_ms,
                        double(stats->gpuTimeEnd - stats->gpuTimeBegin) * to_gpu_ms,
                        stats->maxGpuLatency);

            int64_t used = stats->gpuMemoryUsed;
            int64_t max = stats->gpuMemoryMax;

            if(used > 0 && max > 0)
            {
                char strMax[64];
                bx::prettify(strMax, 64, uint64_t(stats->gpuMemoryUsed));
                //                char tmp1[64];
                //                bx::prettify(tmp1, 64, uint64_t(stats->gpuMemoryMax));
                //                ImGui::Text("GPU mem: %s / %s", tmp0, tmp1);
                //                char tmp2[64];
                //                bx::prettify(tmp2, 64, uint64_t(stats->rtMemoryUsed));
                //                ImGui::Text("Render Target mem: %s / %s", tmp2, tmp0);
                //                char tmp3[64];
                //                bx::prettify(tmp3, 64, uint64_t(stats->textureMemoryUsed));
                //                ImGui::Text("Texture mem: %s / %s", tmp3, tmp0);

                ImGui::Separator();

                // gpu memory
                {
                    char strUsed[64];
                    bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->gpuMemoryUsed);

                    ImGui::Text("GPU mem: %s / %s", strUsed, strMax);
                    ImGui::PlotLines("",
                                     gpuMemoryValues,
                                     IM_ARRAYSIZE(gpuMemoryValues),
                                     (int)offset + 1,
                                     nullptr,
                                     0.0f,
                                     float(max),
                                     ImVec2(overlayWidth, 50));
                }
                ImGui::Separator();

                // rt memory
                {
                    char strUsed[64];
                    bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->rtMemoryUsed);
                    ImGui::Text("Render Target mem: %s / %s", strUsed, strMax);
                    ImGui::PlotLines("",
                                     rtMemoryValues,
                                     IM_ARRAYSIZE(rtMemoryValues),
                                     (int)offset + 1,
                                     nullptr,
                                     0.0f,
                                     float(max),
                                     ImVec2(overlayWidth, 50));
                }
                ImGui::Separator();

                // texture memory
                {
                    char strUsed[64];
                    bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->textureMemoryUsed);
                    ImGui::Text("Texture mem: %s / %s", strUsed, strMax);
                    ImGui::PlotLines("",
                                     textureMemoryValues,
                                     IM_ARRAYSIZE(textureMemoryValues),
                                     (int)offset + 1,
                                     nullptr,
                                     0.0f,
                                     float(max),
                                     ImVec2(overlayWidth, 50));
                }
            }
            else
            {
                ImGui::TextWrapped(ICON_MDI_ALERT " GPU memory data unavailable");
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
        if(ImGui::CollapsingHeader(ICON_MDI_PUZZLE "\tResources"))
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

        if(ImGui::CollapsingHeader(ICON_MDI_CLOCK_OUTLINE "\tProfiler"))
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

    auto& camera_comp = editor_camera.get<camera_component>();
    const auto& camera = camera_comp.get_camera();

    auto p = ImGui::GetItemRectMin();
    auto s = ImGui::GetItemRectSize();
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(p.x, p.y, s.x, s.y);
    ImGuizmo::SetOrthographic(camera.get_projection_mode() == projection_mode::orthographic);
    //    math::mat4 grid(1.0f);
    //    ImGuizmo::DrawGrid(camera.get_view(), camera.get_projection(), math::value_ptr(grid), 100.0f);

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

                if(ImGuizmo::IsScaleType(movetype))
                {
                    APPLOG_INFO("delta S {}", math::to_string(delta.get_scale()));
                    transform_comp.scale_by_local(delta.get_scale());
                }

                if(ImGuizmo::IsRotateType(movetype))
                {
                    APPLOG_INFO("delta R {}", math::to_string(delta.get_rotation_euler_degrees()));
                    transform_comp.rotate_by_local(delta.get_rotation());
                }

                if(ImGuizmo::IsTranslateType(movetype))
                {
                    auto skew = transform_comp.get_skew_local();
                    APPLOG_INFO("delta T {}", math::to_string(delta.get_translation()));
                    transform_comp.move_by_global(delta.get_translation());
                    transform_comp.set_skew_local(skew);
                }
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

                auto object = def.create_mesh_entity_at(ctx,
                                                        key,
                                                        camera_comp.get_camera(),
                                                        math::vec2{cursor_pos.x, cursor_pos.y});
                ;

                es.select(object);
            }
        }

        ImGui::EndDragDropTarget();
    }
}

void scene_panel::init(rtti::context& ctx)
{
}

void scene_panel::draw(rtti::context& ctx)
{
    auto& em = ctx.get<editing_manager>();
    auto& ec = ctx.get<ecs>();

    if(ImGui::BeginMenuBar())
    {
        auto& tm = ctx.get<thumbnail_manager>();

        float width = ImGui::GetContentRegionAvail().x;
        if(ImGui::MenuItem(ICON_MDI_CURSOR_MOVE, nullptr, em.operation == ImGuizmo::OPERATION::TRANSLATE))
        {
            em.operation = ImGuizmo::OPERATION::TRANSLATE;
        }
        if(ImGui::MenuItem(ICON_MDI_ROTATE_3D_VARIANT, nullptr, em.operation == ImGuizmo::OPERATION::ROTATE))
        {
            em.operation = ImGuizmo::OPERATION::ROTATE;
        }

        if(ImGui::MenuItem(ICON_MDI_RELATIVE_SCALE, nullptr, em.operation == ImGuizmo::OPERATION::SCALE))
        {
            em.operation = ImGuizmo::OPERATION::SCALE;
            em.mode = ImGuizmo::MODE::LOCAL;
        }

        if(ImGui::MenuItem(ICON_MDI_MOVE_RESIZE, nullptr, em.operation == ImGuizmo::OPERATION::UNIVERSAL))
        {
            em.operation = ImGuizmo::OPERATION::UNIVERSAL;
            em.mode = ImGuizmo::MODE::LOCAL;
        }

        auto icon = em.mode == ImGuizmo::MODE::LOCAL ? ICON_MDI_CUBE "Local" ICON_MDI_ARROW_DOWN_BOLD
                                                     : ICON_MDI_WEB "Global" ICON_MDI_ARROW_DOWN_BOLD;

        if(ImGui::BeginMenu(icon))
        {
            if(ImGui::MenuItem(ICON_MDI_CUBE "Local", nullptr, em.mode == ImGuizmo::MODE::LOCAL))
            {
                em.mode = ImGuizmo::MODE::LOCAL;
            }

            if(ImGui::MenuItem(ICON_MDI_WEB "Global", nullptr, em.mode == ImGuizmo::MODE::WORLD))
            {
                em.mode = ImGuizmo::MODE::WORLD;
            }
            ImGui::EndMenu();
        }

        if(ImGui::MenuItem(ICON_MDI_GRID, nullptr, em.show_grid))
        {
            em.show_grid = !em.show_grid;
        }
        if(ImGui::BeginMenu(ICON_MDI_ARROW_DOWN_BOLD, em.show_grid))
        {
            ImGui::TextUnformatted("Grid Visual");
            ImGui::LabelText("Grid Plane", "%s", "X Y Z");
            ImGui::EndMenu();
        }


        ImGui::EndMenuBar();
    }

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
            pick_manager.request_pick({pos.x, pos.y}, camera);
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

    process_drag_drop_target(ctx, camera_comp);
}

} // namespace ace
