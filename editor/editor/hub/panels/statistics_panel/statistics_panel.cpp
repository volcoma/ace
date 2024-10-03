#include "statistics_panel.h"
#include "../panels_defs.h"

#include <engine/profiler/profiler.h>

#include <graphics/graphics.h>
#include <math/math.h>

#include <algorithm>
#include <numeric>

namespace ace
{
namespace
{
struct SampleData
{
    static constexpr uint32_t kNumSamples = 500;

    SampleData()
    {
        reset(0.0f);
    }

    void reset(float value)
    {
        m_offset = 0;

        std::fill(std::begin(m_values), std::end(m_values), value);
        // bx::memSet(m_values, 0, sizeof(m_values) );

        m_min = value;
        m_max = value;
        m_avg = value;

        m_smartInitSamples = 20;
    }

    void pushSample(float value)
    {
        if(m_smartInitSamples > 0 && m_offset > m_smartInitSamples)
        {
            reset(value);
            m_smartInitSamples = -1;
        }

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

    int32_t m_smartInitSamples{-1};
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
    ImGui::Text("%s: %6d / %6d", _name, _num, _max);
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

void draw_statistics(bool& enable_profiler)
{
    auto& io = ImGui::GetIO();

    //    auto cursorPos = ImGui::GetCursorScreenPos();
    auto area = ImGui::GetContentRegionAvail();
    //    auto stat_pos = cursorPos + ImVec2(10.0f, 10.0f);
    //    ImGui::SetNextWindowPos(stat_pos);
    //    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), area);

    //    ImGui::SetNextWindowBgAlpha(0.7f);
    //    if(ImGui::BeginChild(ICON_MDI_CHART_BAR "\tSTATISTICS##Scene",
    //                         {},
    //                         ImGuiChildFlags_Border | ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeX |
    //                             ImGuiChildFlags_AutoResizeY,
    //                         ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
    {
        // ImGui::PopStyleColor();
        auto overlayWidth = area.x; // 300.0f;
        auto stats = gfx::get_stats();

        const double to_cpu_ms = 1000.0 / double(stats->cpuTimerFreq);
        const double to_gpu_ms = 1000.0 / double(stats->gpuTimerFreq);
        const double frame_ms = double(stats->cpuTimeFrame) * to_cpu_ms;
        // const double fps = 1000.0f / frame_ms;

        static SampleData frame_time_samples;
        static SampleData graphics_passes_samples;
        ;

        static SampleData gpu_mem_samples;
        static SampleData rt_mem_samples;
        static SampleData texture_mem_samples;

        frame_time_samples.pushSample(float(frame_ms));
        graphics_passes_samples.pushSample(float(gfx::render_pass::get_last_frame_max_pass_id()));
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

        char pasesTextOverlay[256];
        bx::snprintf(pasesTextOverlay,
                     BX_COUNTOF(pasesTextOverlay),
                     "Min: %.0f, Max: %.0f\nAvg: %.0f PASSES",
                     graphics_passes_samples.m_min,
                     graphics_passes_samples.m_max,
                     graphics_passes_samples.m_avg);

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

            ImGui::PlotLines("##Frame",
                             graphics_passes_samples.m_values,
                             SampleData::kNumSamples,
                             graphics_passes_samples.m_offset,
                             pasesTextOverlay,
                             0.0f,
                             200.0f,
                             ImVec2(overlayWidth, 50));

            ImGui::Text("Submit CPU %0.3f, GPU %0.3f (L: %d)",
                        double(stats->cpuTimeEnd - stats->cpuTimeBegin) * to_cpu_ms,
                        double(stats->gpuTimeEnd - stats->gpuTimeBegin) * to_gpu_ms,
                        stats->maxGpuLatency);
            ImGui::Text("Render Passes: %u", gfx::render_pass::get_last_frame_max_pass_id());

            std::uint32_t total_primitives =
                std::accumulate(std::begin(stats->numPrims), std::end(stats->numPrims), 0u);
            std::uint32_t ui_primitives = io.MetricsRenderIndices / 3;
            ui_primitives = std::min(ui_primitives, total_primitives);
            auto scene_primitives = total_primitives - ui_primitives;

            ImGui::Text("Scene Primitives: %u", scene_primitives);
            ImGui::Text("UI    Primitives: %u", ui_primitives);
            ImGui::Text("Total Primitives: %u", total_primitives);

            std::uint32_t ui_draw_calls = ImGui::GetDrawCalls();
            ui_draw_calls = std::min(ui_draw_calls, stats->numDraw);
            auto scene_draw_calls = stats->numDraw - ui_draw_calls;
            ImGui::Text("Scene Draw Calls: %u", scene_draw_calls);
            ImGui::Text("UI    Draw Calls: %u", ui_draw_calls);
            ImGui::Text("Total Draw Calls: %u", stats->numDraw);
            ImGui::Text("Total Comp Calls: %u", stats->numCompute);
            ImGui::Text("Total Blit Calls: %u", stats->numBlit);

            ImGui::PopFont();
        }

        if(ImGui::CollapsingHeader(ICON_MDI_INFORMATION "\tRender Info"))
        {
            ImGui::PushFont(ImGui::Font::Mono);

            auto gpuMemoryMax = stats->gpuMemoryMax;

            // if(stats->gpuMemoryUsed > 0 && gpuMemoryMax > 0)
            {
                // gpu memory
                if(stats->gpuMemoryUsed > 0)
                {
                    gpuMemoryMax = std::max(stats->gpuMemoryUsed, stats->gpuMemoryMax);

                    char strMax[64];
                    bx::prettify(strMax, 64, uint64_t(gpuMemoryMax));

                    char strUsed[64];
                    bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->gpuMemoryUsed);

                    ImGui::Separator();
                    ImGui::Text("GPU mem: %s / %s", strUsed, strMax);
                    ImGui::PlotLines("",
                                     gpu_mem_samples.m_values,
                                     gpu_mem_samples.kNumSamples,
                                     gpu_mem_samples.m_offset,
                                     nullptr,
                                     0.0f,
                                     float(gpuMemoryMax),
                                     ImVec2(overlayWidth, 50));
                }

                // rt memory
                {
                    gpuMemoryMax = std::max(stats->rtMemoryUsed, gpuMemoryMax);

                    char strMax[64];
                    bx::prettify(strMax, 64, uint64_t(gpuMemoryMax));

                    char strUsed[64];
                    bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->rtMemoryUsed);
                    ImGui::Separator();
                    ImGui::Text("Render Target mem: %s / %s", strUsed, strMax);
                    ImGui::PlotLines("",
                                     rt_mem_samples.m_values,
                                     rt_mem_samples.kNumSamples,
                                     rt_mem_samples.m_offset,
                                     nullptr,
                                     0.0f,
                                     float(gpuMemoryMax),
                                     ImVec2(overlayWidth, 50));
                }

                // texture memory
                {
                    gpuMemoryMax = std::max(stats->textureMemoryUsed, gpuMemoryMax);

                    char strMax[64];
                    bx::prettify(strMax, 64, uint64_t(gpuMemoryMax));

                    char strUsed[64];
                    bx::prettify(strUsed, BX_COUNTOF(strUsed), stats->textureMemoryUsed);
                    ImGui::Separator();
                    ImGui::Text("Texture mem: %s / %s", strUsed, strMax);
                    ImGui::PlotLines("",
                                     texture_mem_samples.m_values,
                                     texture_mem_samples.kNumSamples,
                                     texture_mem_samples.m_offset,
                                     nullptr,
                                     0.0f,
                                     float(gpuMemoryMax),
                                     ImVec2(overlayWidth, 50));
                }
            }
            // else
            // {
            //     ImGui::TextWrapped(ICON_MDI_ALERT " GPU memory data unavailable");
            // }

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
            if(ImGui::Checkbox("Enable GPU profiler", &enable_profiler))
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

            {
                auto profiler = get_app_profiler();
                const auto& data = profiler->get_per_frame_data_read();

                for(const auto& [name, perFrameData] : data)
                {
                    ImGui::TextUnformatted(
                        fmt::format("{:>7.3f}ms [{:^5}] - {}", perFrameData.time, perFrameData.samples, name).c_str());
                }
            }
            ImGui::PopFont();
        }
    }
    //    ImGui::EndChild();
}

} // namespace
void statistics_panel::init(rtti::context& ctx)
{
}

void statistics_panel::deinit(rtti::context& ctx)
{
}

void statistics_panel::on_frame_update(rtti::context& ctx, delta_t dt)
{
}

void statistics_panel::on_frame_render(rtti::context& ctx, delta_t dt)
{
}

void statistics_panel::on_frame_ui_render(rtti::context& ctx, const char* name)
{
    if(ImGui::Begin(name, nullptr, ImGuiWindowFlags_MenuBar))
    {
        draw_menubar(ctx);
        draw_statistics(enable_profiler_);
    }
    ImGui::End();
}

void statistics_panel::draw_menubar(rtti::context& ctx)
{
    if(ImGui::BeginMenuBar())
    {
        ImGui::EndMenuBar();
    }
}

} // namespace ace
