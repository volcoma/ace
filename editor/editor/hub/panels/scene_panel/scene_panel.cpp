#include "scene_panel.h"
#include "imgui/imgui.h"
#include "logging/logging.h"
#include "math/transform.hpp"
#include <imgui/imgui_internal.h>
#include <imgui_widgets/gizmo.h>

#include <editor/ecs/editor_ecs.h>
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
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/rendering/renderer.h>

#include <algorithm>
#include <filesystem/filesystem.h>
#include <numeric>
namespace ace
{

struct SampleData
{
    static constexpr uint32_t kNumSamples = 500;

    SampleData()
    {
        reset(0.0f);
    }

    SampleData(float value)
    {
        reset(value);
    }

    void reset(float value)
    {
        m_offset = 0;

        std::fill(std::begin(m_values), std::end(m_values), value);
        // bx::memSet(m_values, 0, sizeof(m_values) );

        m_min = value;
        m_max = value;
        m_avg = value;
    }

    void pushSample(float value)
    {
        m_values[m_offset] = value;
        m_offset = (m_offset + 1) % kNumSamples;

        float min = bx::max<float>();
        float max = bx::min<float>();
        float avg = 0.0f;

        for(uint32_t ii = 0; ii < kNumSamples; ++ii)
        {
            const float val = m_values[ii];
            min = bx::min(min, val);
            max = bx::max(max, val);
            avg += val;
        }

        m_min = min;
        m_max = max;
        m_avg = avg / kNumSamples;
    }

    int32_t m_offset{};
    float m_values[kNumSamples]{};

    float m_min{};
    float m_max{};
    float m_avg{};
};

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

    auto cursorPos = ImGui::GetCursorScreenPos();
    auto area = ImGui::GetContentRegionAvail();
    auto stat_pos = cursorPos + ImVec2(10.0f, 10.0f);
    ImGui::SetNextWindowPos(stat_pos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), area);

    ImGui::SetNextWindowBgAlpha(0.7f);
    if(ImGui::BeginChild(ICON_MDI_CHART_BAR "\tSTATISTICS##Scene",
                         {},
                         ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX |
                             ImGuiChildFlags_AutoResizeY,
                         ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
    {
        // ImGui::PopStyleColor();
        auto overlayWidth = 300.0f;
        auto stats = gfx::get_stats();

        const double to_cpu_ms = 1000.0 / double(stats->cpuTimerFreq);
        const double to_gpu_ms = 1000.0 / double(stats->gpuTimerFreq);
        const double frame_ms = double(stats->cpuTimeFrame) * to_cpu_ms;
        const double fps = 1000.0f / frame_ms;

        static SampleData frame_time_samples{float(frame_ms)};
        static SampleData gpu_mem_samples{float(stats->gpuMemoryUsed) / 1024 / 1024};
        static SampleData rt_mem_samples{float(stats->rtMemoryUsed) / 1024 / 1024};
        static SampleData texture_mem_samples{float(stats->textureMemoryUsed) / 1024 / 1024};

        frame_time_samples.pushSample(float(frame_ms));
        gpu_mem_samples.pushSample(float(stats->gpuMemoryUsed) / 1024 / 1024);
        rt_mem_samples.pushSample(float(stats->rtMemoryUsed) / 1024 / 1024);
        texture_mem_samples.pushSample(float(stats->textureMemoryUsed) / 1024 / 1024);

        char frameTextOverlay[256];
        bx::snprintf(frameTextOverlay,
                     BX_COUNTOF(frameTextOverlay),
                     "Min: %.3fms, Max: %.3fms\nAvg: %.3fms, %.1f FPS",
                     frame_time_samples.m_min,
                     frame_time_samples.m_max,
                     frame_time_samples.m_avg,
                     1000.0f / frame_time_samples.m_avg);
        {
            ImGui::PushFont(ImGui::Font::Mono);

            ImGui::PlotLines("##Frame",
                             frame_time_samples.m_values,
                             SampleData::kNumSamples,
                             frame_time_samples.m_offset,
                             frameTextOverlay,
                             0.0f,
                             200.0f,
                             ImVec2(overlayWidth, 50));

            ImGui::Text("Submit CPU %0.3f, GPU %0.3f (L: %d)",
                        double(stats->cpuTimeEnd - stats->cpuTimeBegin) * to_cpu_ms,
                        double(stats->gpuTimeEnd - stats->gpuTimeBegin) * to_gpu_ms,
                        stats->maxGpuLatency);

            std::uint32_t ui_draw_calls = ImGui::GetDrawCalls();
            std::uint32_t ui_primitives = io.MetricsRenderIndices / 3;
            std::uint32_t total_primitives =
                std::accumulate(std::begin(stats->numPrims), std::end(stats->numPrims), 0u);
            ImGui::Text("Scene Primitives: %u", math::abs<std::uint32_t>(total_primitives - ui_primitives));
            // ImGui::Text("UI    Primitives: %u", ui_primitives);
            // ImGui::Text("Total Primitives: %u", total_primitives);

            ImGui::Text("Scene Draw Calls: %u", math::abs<std::uint32_t>(stats->numDraw - ui_draw_calls));
            // ImGui::Text("UI    Draw Calls: %u", ui_draw_calls);
            // ImGui::Text("Total Draw Calls: %u", stats->numDraw);
            ImGui::PopFont();
        }

        if(ImGui::CollapsingHeader(ICON_MDI_INFORMATION "\tRender Info"))
        {
            ImGui::PushFont(ImGui::Font::Mono);

            int64_t used = stats->gpuMemoryUsed;
            int64_t max = stats->gpuMemoryMax;

            if(used > 0 && max > 0)
            {
                char strMax[64];
                bx::prettify(strMax, 64, uint64_t(stats->gpuMemoryUsed));
                ImGui::Separator();

                // gpu memory
                {
                    char strUsed[64];
                    bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->gpuMemoryUsed);

                    ImGui::Text("GPU mem: %s / %s", strUsed, strMax);
                    ImGui::PlotLines("",
                                     gpu_mem_samples.m_values,
                                     gpu_mem_samples.kNumSamples,
                                     gpu_mem_samples.m_offset,
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
                                     rt_mem_samples.m_values,
                                     rt_mem_samples.kNumSamples,
                                     rt_mem_samples.m_offset,
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
                                     texture_mem_samples.m_values,
                                     texture_mem_samples.kNumSamples,
                                     texture_mem_samples.m_offset,
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
    ImGui::EndChild();
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
                             ImGui::GetColorU32(ImGuiCol_Button));
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

                auto object = def.create_mesh_entity_at(ctx,
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

                auto object =
                    def.create_prefab_at(ctx, key, camera_comp.get_camera(), math::vec2{cursor_pos.x, cursor_pos.y});

                es.select(object);
            }
        }

        ImGui::EndDragDropTarget();
    }
}

void scene_panel::draw_menubar(rtti::context& ctx)
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

            auto& eecs = ctx.get<editor_ecs>();
            auto& camera_comp = eecs.editor_camera.get<camera_component>();
            inspect(ctx, camera_comp);

            ImGui::EndMenu();
        }
        ImGui::SetItemTooltip("%s", "Settings for the Scene view camera.");

        {
            auto threads = itc::get_all_registered_threads();
            size_t total_jobs = 0;
            for(const auto& id : threads)
            {
                total_jobs += itc::get_pending_task_count(id);
            }

            auto& thr = ctx.get<threader>();
            auto pool_jobs = thr.pool->get_jobs_count();

            if(ImGui::BeginMenu(fmt::format("{}{}###jobs", ICON_MDI_BUS_ALERT, total_jobs).c_str()))
            {
                ImGui::TextUnformatted(
                    fmt::format("Threads : {}, Jobs : {}, Pool Jobs {}", threads.size(), total_jobs, pool_jobs)
                        .c_str());
                for(const auto& id : threads)
                {
                    auto jobs_info = itc::get_pending_task_count_detailed(id);
                    ImGui::TextUnformatted(
                        fmt::format("Thread : {}, Jobs : {}", jobs_info.thread_name, jobs_info.count).c_str());
                }
                ImGui::EndMenu();
            }
            ImGui::SetItemTooltip("%s", "Show/Hide Jobs");
        }

        auto icon_size = ImGui::CalcTextSize(ICON_MDI_CHART_BAR).x;

        ImGui::PushFont(ImGui::Font::Mono);
        auto& io = ImGui::GetIO();
        auto fps_size = ImGui::CalcTextSize(fmt::format("{:.1f}", io.Framerate).c_str()).x;
        ImGui::PopFont();

        ImGui::AlignedItem(1.0f,
                           ImGui::GetContentRegionAvail().x,
                           icon_size + fps_size + ImGui::GetStyle().ItemSpacing.x * 2.0f,
                           [&]()
                           {
                               if(ImGui::MenuItem(ICON_MDI_CHART_BAR, nullptr, show_statistics_))
                               {
                                   show_statistics_ = !show_statistics_;
                               }
                               ImGui::SetItemTooltip("%s", "Show/Hide Stats");
                               ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.x * 2.0f);
                               ImGui::PushFont(ImGui::Font::Mono);
                               ImGui::Text("%.1f", io.Framerate);
                               ImGui::PopFont();
                           });

        ImGui::EndMenuBar();
    }
}

void scene_panel::init(rtti::context& ctx)
{
}

void scene_panel::draw(rtti::context& ctx)
{
    draw_menubar(ctx);

    auto& em = ctx.get<editing_manager>();
    auto& eecs = ctx.get<editor_ecs>();

    auto& rend = ctx.get<renderer>();

    auto& editor_camera = eecs.editor_camera;

    bool has_edit_camera = editor_camera && editor_camera.all_of<transform_component, camera_component>();

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

    if(show_statistics_)
    {
        ImGui::SetCursorScreenPos(pos);
        show_statistics(show_gbuffer_, enable_profiler_);
    }
}

} // namespace ace
