#pragma once
#include <engine/rendering/render_window.h>
#include <imgui/imgui.h> // IMGUI_IMPL_API

using ImGui_ImplOSPP_RenderWindow_Callback =
    std::function<void(ace::render_window* window, ImGuiViewport* viewport, void*)>;
using ImGui_ImplOSPP_SwapBuffers_Callback =
    std::function<void(ace::render_window* window, ImGuiViewport* viewport, void*)>;

IMGUI_IMPL_API auto ImGui_ImplOSPP_Init(ace::render_window* window,
                                        ImGui_ImplOSPP_RenderWindow_Callback render_callback,
                                        ImGui_ImplOSPP_SwapBuffers_Callback swap_callback) -> bool;

IMGUI_IMPL_API void ImGui_ImplOSPP_Shutdown();
IMGUI_IMPL_API void ImGui_ImplOSPP_NewFrame(float delta_time);
IMGUI_IMPL_API void ImGui_ImplOSPP_EndFrame();

IMGUI_IMPL_API auto ImGui_ImplOSPP_ProcessEvent(const os::event* event) -> bool;
