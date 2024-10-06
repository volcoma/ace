#include "imgui_impl_ospp.h"
#include <ospp/clipboard.h>
#include <ospp/display_mode.h>
#include <ospp/hints.h>

#include <utility>
// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion" // warning: implicit conversion from 'xxx'
                                                                   // to 'float' may lose precision
#endif

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS) &&                     \
    !defined(__amigaos4__)
#define OSPP_HAS_CAPTURE_AND_GLOBAL_MOUSE 1
#else
#define OSPP_HAS_CAPTURE_AND_GLOBAL_MOUSE 0
#endif

// OSPP Data
struct ImGui_ImplOSPP_Data
{
    ace::render_window* Window{};
    uint32_t MouseWindowID{};
    int MouseButtonsDown{};
    ImGuiMouseCursor LastMouseCursor{ImGuiMouseCursor_COUNT};
    int PendingMouseLeaveFrame{};
    std::string ClipboardTextData{};
    bool MouseCanUseGlobalState{};
    bool MouseCanReportHoveredViewport{}; // This is hard to use/unreliable on OSPP so we'll set
                                          // ImGuiBackendFlags_HasMouseHoveredViewport dynamically based on
                                          // state.
    bool WantUpdateMonitors{};
    bool NoMonitorDetected{};

    ImGui_ImplOSPP_RenderWindow_Callback RenderCallback;
    ImGui_ImplOSPP_SwapBuffers_Callback SwapCallback;
};

// Helper structure we store in the void* RendererUserData field of each ImGuiViewport to easily retrieve our
// backend data.
struct ImGui_ImplOSPP_ViewportData
{
    ace::render_window* Window;
    bool WindowOwned;

    ImGui_ImplOSPP_ViewportData()
    {
        Window = nullptr;
        WindowOwned = false;
    }
    ~ImGui_ImplOSPP_ViewportData()
    {
        IM_ASSERT(Window == nullptr);
    }
};

static auto ImGui_ImplOSPP_GetViewportData(ImGuiViewport* viewport) -> ImGui_ImplOSPP_ViewportData*
{
    if(viewport && viewport->PlatformUserData)
    {
        auto data = (ImGui_ImplOSPP_ViewportData*)viewport->PlatformUserData;
        return data;
    }

    return nullptr;
}

static void ImGui_ImplOSPP_ForEachViewport(
    const std::function<void(ImGuiViewport* viewport, ImGui_ImplOSPP_ViewportData* data)>& callback)
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();

    for(int i = 0; i != platform_io.Viewports.Size; i++)
    {
        auto data = ImGui_ImplOSPP_GetViewportData(platform_io.Viewports[i]);
        if(data)
        {
            callback(platform_io.Viewports[i], data);
        }
    }
}

static auto ImGui_ImplOSPP_GetViewportFromWindowId(uint32_t id) -> ImGuiViewport*
{
    ImGuiViewport* result{nullptr};
    ImGui_ImplOSPP_ForEachViewport(
        [&](ImGuiViewport* viewport, ImGui_ImplOSPP_ViewportData* data)
        {
            if(!result)
            {
                if(data->Window && data->Window->get_window().get_id() == id)
                {
                    result = viewport;
                }
            }
        });

    return result;
}

static auto ImGui_ImplOSPP_GetFocusedViewport() -> ImGuiViewport*
{
    ImGuiViewport* result{nullptr};
    ImGui_ImplOSPP_ForEachViewport(
        [&](ImGuiViewport* viewport, ImGui_ImplOSPP_ViewportData* data)
        {
            if(!result && data->Window->get_window().has_focus())
            {
                result = viewport;
            }
        });

    return result;
}

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context +
// multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
// FIXME: some shared resources (mouse cursor shape, gamepad) are mishandled when using multi-context.
static auto ImGui_ImplOSPP_GetBackendData() -> ImGui_ImplOSPP_Data*
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplOSPP_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

auto ImGui_ImplOSPP_IdToHandle(uint32_t id) -> void*
{
    return reinterpret_cast<void*>(uintptr_t(id));
}

auto ImGui_ImplOSPP_MapCursor(ImGuiMouseCursor cursor) -> os::cursor::type
{
    static const std::map<ImGuiMouseCursor, os::cursor::type> cursor_map = {
        {ImGuiMouseCursor_Arrow, os::cursor::type::arrow},
        {ImGuiMouseCursor_TextInput, os::cursor::type::ibeam},
        {ImGuiMouseCursor_ResizeNS, os::cursor::type::size_ns},
        {ImGuiMouseCursor_ResizeEW, os::cursor::type::size_we},
        {ImGuiMouseCursor_ResizeNESW, os::cursor::type::size_nesw},
        {ImGuiMouseCursor_ResizeNWSE, os::cursor::type::size_nwse},
        {ImGuiMouseCursor_ResizeAll, os::cursor::type::size_all},
        {ImGuiMouseCursor_Hand, os::cursor::type::hand},
        //        {ImGuiMouseCursor_Help, os::cursor::type::hand},
        //        {ImGuiMouseCursor_Wait, os::cursor::type::wait},
        //        {ImGuiMouseCursor_ArrowWait, os::cursor::type::wait},
        //        {ImGuiMouseCursor_Cross, os::cursor::type::crosshair},
        {ImGuiMouseCursor_NotAllowed, os::cursor::type::not_allowed}};
    auto it = cursor_map.find(cursor);
    if(it != cursor_map.end())
    {
        return it->second;
    }
    return os::cursor::type::arrow;
}

// Forward Declarations
static void ImGui_ImplOSPP_UpdateMonitors();
static void ImGui_ImplOSPP_InitPlatformInterface(ace::render_window* window);
static void ImGui_ImplOSPP_ShutdownPlatformInterface();

// Functions
static auto ImGui_ImplOSPP_GetClipboardText(ImGuiContext* ctx) -> const char*
{
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();

    bd->ClipboardTextData = os::clipboard::get_text();
    return bd->ClipboardTextData.c_str();
}

static void ImGui_ImplOSPP_SetClipboardText(ImGuiContext* ctx, const char* text)
{
    os::clipboard::set_text(text);
}

static void ImGui_ImplOSPP_SetPlatformImeData(ImGuiContext* ctx, ImGuiViewport* viewport, ImGuiPlatformImeData* data)
{
    if(data->WantVisible)
    {
        //        OSPP_Rect r;
        //        r.x = (int)(data->InputPos.x - viewport->Pos.x);
        //        r.y = (int)(data->InputPos.y - viewport->Pos.y + data->InputLineHeight);
        //        r.w = 1;
        //        r.h = (int)data->InputLineHeight;
        //        OSPP_SetTextInputRect(&r);
    }
}

static auto ImGui_ImplOSPP_KeycodeToImGuiKey(os::key::code keycode) -> ImGuiKey
{
    switch(keycode)
    {
        case os::key::code::tab:
            return ImGuiKey_Tab;
        case os::key::code::left:
            return ImGuiKey_LeftArrow;
        case os::key::code::right:
            return ImGuiKey_RightArrow;
        case os::key::code::up:
            return ImGuiKey_UpArrow;
        case os::key::code::down:
            return ImGuiKey_DownArrow;
        case os::key::code::pageup:
            return ImGuiKey_PageUp;
        case os::key::code::pagedown:
            return ImGuiKey_PageDown;
        case os::key::code::home:
            return ImGuiKey_Home;
        case os::key::code::end:
            return ImGuiKey_End;
        case os::key::code::insert:
            return ImGuiKey_Insert;
        case os::key::code::del:
            return ImGuiKey_Delete;
        case os::key::code::backspace:
            return ImGuiKey_Backspace;
        case os::key::code::space:
            return ImGuiKey_Space;
        case os::key::code::enter:
            return ImGuiKey_Enter;
        case os::key::code::escape:
            return ImGuiKey_Escape;
        case os::key::code::apostrophe:
            return ImGuiKey_Apostrophe;
        case os::key::code::comma:
            return ImGuiKey_Comma;
        case os::key::code::minus:
            return ImGuiKey_Minus;
        case os::key::code::period:
            return ImGuiKey_Period;
        case os::key::code::slash:
            return ImGuiKey_Slash;
        case os::key::code::semicolon:
            return ImGuiKey_Semicolon;
        case os::key::code::equals:
            return ImGuiKey_Equal;
        case os::key::code::leftbracket:
            return ImGuiKey_LeftBracket;
        case os::key::code::backslash:
            return ImGuiKey_Backslash;
        case os::key::code::rightbracket:
            return ImGuiKey_RightBracket;
            //        case os::key::code::BACKQUOTE: return ImGuiKey_GraveAccent;
        case os::key::code::capslock:
            return ImGuiKey_CapsLock;
        case os::key::code::scrolllock:
            return ImGuiKey_ScrollLock;
        case os::key::code::numlockclear:
            return ImGuiKey_NumLock;
        case os::key::code::printscreen:
            return ImGuiKey_PrintScreen;
        case os::key::code::pause:
            return ImGuiKey_Pause;
        case os::key::code::kp_digit0:
            return ImGuiKey_Keypad0;
        case os::key::code::kp_digit1:
            return ImGuiKey_Keypad1;
        case os::key::code::kp_digit2:
            return ImGuiKey_Keypad2;
        case os::key::code::kp_digit3:
            return ImGuiKey_Keypad3;
        case os::key::code::kp_digit4:
            return ImGuiKey_Keypad4;
        case os::key::code::kp_digit5:
            return ImGuiKey_Keypad5;
        case os::key::code::kp_digit6:
            return ImGuiKey_Keypad6;
        case os::key::code::kp_digit7:
            return ImGuiKey_Keypad7;
        case os::key::code::kp_digit8:
            return ImGuiKey_Keypad8;
        case os::key::code::kp_digit9:
            return ImGuiKey_Keypad9;
        case os::key::code::kp_period:
            return ImGuiKey_KeypadDecimal;
        case os::key::code::kp_divide:
            return ImGuiKey_KeypadDivide;
        case os::key::code::kp_multiply:
            return ImGuiKey_KeypadMultiply;
        case os::key::code::kp_minus:
            return ImGuiKey_KeypadSubtract;
        case os::key::code::kp_plus:
            return ImGuiKey_KeypadAdd;
        case os::key::code::kp_enter:
            return ImGuiKey_KeypadEnter;
        case os::key::code::kp_equals:
            return ImGuiKey_KeypadEqual;
        case os::key::code::lctrl:
            return ImGuiKey_LeftCtrl;
        case os::key::code::lshift:
            return ImGuiKey_LeftShift;
        case os::key::code::lalt:
            return ImGuiKey_LeftAlt;
        case os::key::code::lgui:
            return ImGuiKey_LeftSuper;
        case os::key::code::rctrl:
            return ImGuiKey_RightCtrl;
        case os::key::code::rshift:
            return ImGuiKey_RightShift;
        case os::key::code::ralt:
            return ImGuiKey_RightAlt;
        case os::key::code::rgui:
            return ImGuiKey_RightSuper;
        case os::key::code::application:
            return ImGuiKey_Menu;
        case os::key::code::digit0:
            return ImGuiKey_0;
        case os::key::code::digit1:
            return ImGuiKey_1;
        case os::key::code::digit2:
            return ImGuiKey_2;
        case os::key::code::digit3:
            return ImGuiKey_3;
        case os::key::code::digit4:
            return ImGuiKey_4;
        case os::key::code::digit5:
            return ImGuiKey_5;
        case os::key::code::digit6:
            return ImGuiKey_6;
        case os::key::code::digit7:
            return ImGuiKey_7;
        case os::key::code::digit8:
            return ImGuiKey_8;
        case os::key::code::digit9:
            return ImGuiKey_9;
        case os::key::code::a:
            return ImGuiKey_A;
        case os::key::code::b:
            return ImGuiKey_B;
        case os::key::code::c:
            return ImGuiKey_C;
        case os::key::code::d:
            return ImGuiKey_D;
        case os::key::code::e:
            return ImGuiKey_E;
        case os::key::code::f:
            return ImGuiKey_F;
        case os::key::code::g:
            return ImGuiKey_G;
        case os::key::code::h:
            return ImGuiKey_H;
        case os::key::code::i:
            return ImGuiKey_I;
        case os::key::code::j:
            return ImGuiKey_J;
        case os::key::code::k:
            return ImGuiKey_K;
        case os::key::code::l:
            return ImGuiKey_L;
        case os::key::code::m:
            return ImGuiKey_M;
        case os::key::code::n:
            return ImGuiKey_N;
        case os::key::code::o:
            return ImGuiKey_O;
        case os::key::code::p:
            return ImGuiKey_P;
        case os::key::code::q:
            return ImGuiKey_Q;
        case os::key::code::r:
            return ImGuiKey_R;
        case os::key::code::s:
            return ImGuiKey_S;
        case os::key::code::t:
            return ImGuiKey_T;
        case os::key::code::u:
            return ImGuiKey_U;
        case os::key::code::v:
            return ImGuiKey_V;
        case os::key::code::w:
            return ImGuiKey_W;
        case os::key::code::x:
            return ImGuiKey_X;
        case os::key::code::y:
            return ImGuiKey_Y;
        case os::key::code::z:
            return ImGuiKey_Z;
        case os::key::code::f1:
            return ImGuiKey_F1;
        case os::key::code::f2:
            return ImGuiKey_F2;
        case os::key::code::f3:
            return ImGuiKey_F3;
        case os::key::code::f4:
            return ImGuiKey_F4;
        case os::key::code::f5:
            return ImGuiKey_F5;
        case os::key::code::f6:
            return ImGuiKey_F6;
        case os::key::code::f7:
            return ImGuiKey_F7;
        case os::key::code::f8:
            return ImGuiKey_F8;
        case os::key::code::f9:
            return ImGuiKey_F9;
        case os::key::code::f10:
            return ImGuiKey_F10;
        case os::key::code::f11:
            return ImGuiKey_F11;
        case os::key::code::f12:
            return ImGuiKey_F12;
        default:
            return ImGuiKey_None;
    }
}

static void ImGui_ImplOSPP_UpdateKeyModifiers(os::key_event e)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, e.ctrl);
    io.AddKeyEvent(ImGuiMod_Shift, e.shift);
    io.AddKeyEvent(ImGuiMod_Alt, e.alt);
    io.AddKeyEvent(ImGuiMod_Super, e.system);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your
// inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
// clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or
// clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear imgui, and
// hide them from your application based on those two flags. If you have multiple OSPP events and some of them
// are not meant to be used by dear imgui, you may need to filter events based on their windowID field.
auto ImGui_ImplOSPP_ProcessEvent(const os::event* event) -> bool
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();

    switch(event->type)
    {
        case os::events::mouse_motion:
        {
            os::point mouse_pos(event->motion.x, event->motion.y);
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                auto viewport = ImGui_ImplOSPP_GetViewportFromWindowId(event->motion.window_id);
                auto vd = ImGui_ImplOSPP_GetViewportData(viewport);
                if(vd)
                {
                    auto window_pos = vd->Window->get_window().get_position();
                    mouse_pos.x += window_pos.x;
                    mouse_pos.y += window_pos.y;
                }
            }

            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMousePosEvent(float(mouse_pos.x), float(mouse_pos.y));
            return true;
        }
        case os::events::mouse_wheel:
        {
            // IMGUI_DEBUG_LOG("wheel %.2f %.2f, precise %.2f %.2f\n", (float)event->wheel.x,
            // (float)event->wheel.y, event->wheel.preciseX, event->wheel.preciseY);
            float wheel_x = -float(event->wheel.x);
            float wheel_y = float(event->wheel.y);
#ifdef __EMSCRIPTEN__
            wheel_x /= 100.0f;
#endif
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseWheelEvent(wheel_x, wheel_y);
            return true;
        }
        case os::events::mouse_button:
        {
            int mouse_button = int(uint8_t(event->button.button)) - 1;
            if(mouse_button == -1)
                break;

            bool pressed = event->button.state_id == os::state::pressed;
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(mouse_button, pressed);
            bd->MouseButtonsDown =
                pressed ? (bd->MouseButtonsDown | (1 << mouse_button)) : (bd->MouseButtonsDown & ~(1 << mouse_button));
            return true;
        }
        case os::events::text_input:
        {
            io.AddInputCharactersUTF8(event->text.text.c_str());
            return true;
        }
        case os::events::key_up:
        case os::events::key_down:
        {
            ImGui_ImplOSPP_UpdateKeyModifiers(event->key);
            ImGuiKey key = ImGui_ImplOSPP_KeycodeToImGuiKey(event->key.code);
            io.AddKeyEvent(key, (event->type == os::events::key_down));

            io.SetKeyEventNativeData(key, event->key.code, event->key.code, event->key.code);
            return true;
        }
        case os::events::window:
        {
            switch(event->window.type)
            {
                case os::window_event_id::focus_gained:
                {
                    io.AddFocusEvent(true);
                    return true;
                }
                case os::window_event_id::focus_lost:
                {
                    io.AddFocusEvent(false);
                    return true;
                }
                case os::window_event_id::enter:
                {
                    bd->MouseWindowID = event->window.window_id;
                    bd->PendingMouseLeaveFrame = 0;

                    return true;
                }

                case os::window_event_id::leave:
                {
                    if(bd->MouseWindowID == event->window.window_id)
                    {
                        bd->PendingMouseLeaveFrame = ImGui::GetFrameCount() + 1;
                    }
                    return true;
                }

                case os::window_event_id::close:
                case os::window_event_id::moved:
                case os::window_event_id::resized:
                    if(ImGuiViewport* viewport = ImGui_ImplOSPP_GetViewportFromWindowId(event->window.window_id))
                    {
                        if(event->window.type == os::window_event_id::close)
                            viewport->PlatformRequestClose = true;
                        if(event->window.type == os::window_event_id::moved)
                            viewport->PlatformRequestMove = true;
                        if(event->window.type == os::window_event_id::resized)
                            viewport->PlatformRequestResize = true;
                        return true;
                    }
                    return true;
                default:
                    break;
            }
            return true;
        }
        case os::events::display_orientation:
        case os::events::display_connected:
        case os::events::display_disconnected:
        case os::events::display_moved:
        case os::events::display_content_scale_changed:
        {
            bd->WantUpdateMonitors = true;
            return true;
        }

        default:
            break;
    }

    return false;
}

auto ImGui_ImplOSPP_Init(ace::render_window* window,
                         ImGui_ImplOSPP_RenderWindow_Callback render_callback,
                         ImGui_ImplOSPP_SwapBuffers_Callback swap_callback) -> bool
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Check and store if we are on a OSPP backend that supports global mouse position
    // ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a black-list)
    bool mouse_can_use_global_state = false;
#if OSPP_HAS_CAPTURE_AND_GLOBAL_MOUSE
    //    const char* OSPP_backend = OSPP_GetCurrentVideoDriver();
    //    const char* global_mouse_whitelist[] = { "windows", "cocoa", "x11", "DIVE", "VMAN" };
    //    for (int n = 0; n < IM_ARRAYSIZE(global_mouse_whitelist); n++)
    //        if (strncmp(OSPP_backend, global_mouse_whitelist[n], strlen(global_mouse_whitelist[n])) == 0)
    mouse_can_use_global_state = true;
#endif

    // Setup backend capabilities flags
    ImGui_ImplOSPP_Data* bd = IM_NEW(ImGui_ImplOSPP_Data)();
    io.BackendPlatformUserData = bd;
    io.BackendPlatformName = "imgui_impl_ospp";

    // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigDebugHighlightIdConflicts = true;
    // We can create multi-viewports on the
    // Platform side (optional)
    if(mouse_can_use_global_state)
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

    bd->RenderCallback = std::move(render_callback);
    bd->SwapCallback = std::move(swap_callback);
    bd->Window = window;

    // OSPP on Linux/OSX doesn't report events for unfocused windows (see
    // https://github.com/ocornut/imgui/issues/4960) We will use 'MouseCanReportHoveredViewport' to set
    // 'ImGuiBackendFlags_HasMouseHoveredViewport' dynamically each frame.
    bd->MouseCanUseGlobalState = mouse_can_use_global_state;
#ifndef __APPLE__
    bd->MouseCanReportHoveredViewport = bd->MouseCanUseGlobalState;
#else
    bd->MouseCanReportHoveredViewport = false;
#endif
    bd->WantUpdateMonitors = true;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_SetClipboardTextFn = ImGui_ImplOSPP_SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = ImGui_ImplOSPP_GetClipboardText;
    platform_io.Platform_ClipboardUserData = nullptr;
    platform_io.Platform_SetImeDataFn = ImGui_ImplOSPP_SetPlatformImeData;

    // Set platform dependent data in viewport
    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();

    main_viewport->PlatformHandle = ImGui_ImplOSPP_IdToHandle(window->get_window().get_id());
    main_viewport->PlatformHandleRaw = window->get_window().get_native_handle();

    // From 2.0.5: Set SDL hint to receive mouse click events on window focus, otherwise SDL doesn't emit the
    // event. Without this, when clicking to gain focus, our widgets wouldn't activate even though they showed
    // as hovered. (This is unfortunately a global SDL setting, so enabling it might have a side-effect on
    // your application. It is unlikely to make a difference, but if your app absolutely needs to ignore the
    // initial on-focus click: you can ignore OSPP_EVENT_MOUSE_BUTTON_DOWN events coming right after a
    // OSPP_WINDOWEVENT_FOCUS_GAINED)
    os::set_hint("HINT_MOUSE_FOCUS_CLICKTHROUGH", "1");

    // From 2.0.22: Disable auto-capture, this is preventing drag and drop across multiple windows (see #5710)
    os::set_hint("HINT_MOUSE_AUTO_CAPTURE", "0");

    // SDL 3.x : see https://github.com/libSDL-org/SDL/issues/6659
    os::set_hint("HINT_BORDERLESS_WINDOWED_STYLE", "0");

    if((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) &&
       (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports))
        ImGui_ImplOSPP_InitPlatformInterface(window);

    return true;
}

void ImGui_ImplOSPP_Shutdown()
{
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOSPP_ShutdownPlatformInterface();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &=
        ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad |
          ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_HasMouseHoveredViewport);
    IM_DELETE(bd);
}

// This code is incredibly messy because some of the functions we need for full viewport support are not
// available in OSPP < 2.0.4.
static void ImGui_ImplOSPP_UpdateMouseData()
{
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    // We forward mouse input when hovered or captured (via OSPP_EVENT_MOUSE_MOTION) or when focused (below)
#if OSPP_HAS_CAPTURE_AND_GLOBAL_MOUSE
    // OSPP_CaptureMouse() let the OS know e.g. that our imgui drag outside the OSPP window boundaries
    // shouldn't
    os::mouse::capture((bd->MouseButtonsDown != 0));

    // e.g. trigger other operations outside
    ImGuiViewport* focused_viewport = ImGui_ImplOSPP_GetFocusedViewport();

    const bool is_app_focused = !!focused_viewport;
#else
    const bool is_app_focused =
        bd->Window->get_window().has_focus(); // OSPP 2.0.3 and non-windowed systems: single-viewport only
#endif
    if(is_app_focused)
    {
        // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when
        // ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        if(io.WantSetMousePos)
        {
            os::point mouse_pos(int32_t(io.MousePos.x), int32_t(io.MousePos.y));
#if OSPP_HAS_CAPTURE_AND_GLOBAL_MOUSE
            if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
                os::mouse::set_position(mouse_pos);
            else
#endif
                os::mouse::set_position(mouse_pos, bd->Window->get_window());
        }

        // (Optional) Fallback to provide mouse position when focused (OSPP_EVENT_MOUSE_MOTION already
        // provides this when hovered or captured)
        if(bd->MouseCanUseGlobalState && bd->MouseButtonsDown == 0)
        {
            // Single-viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when
            // the mouse is on the upper-left corner of the app window) Multi-viewport mode: mouse position in
            // OS absolute coordinates (io.MousePos is (0,0) when the mouse is on the upper-left of the
            // primary monitor)
            auto mouse_pos = os::mouse::get_position();
            if(!(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
            {
                auto fvd = ImGui_ImplOSPP_GetViewportData(focused_viewport);
                if(fvd)
                {
                    mouse_pos = os::mouse::get_position(fvd->Window->get_window());
                }
            }
            io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
            io.AddMousePosEvent(float(mouse_pos.x), float(mouse_pos.y));
        }
    }

    // (Optional) When using multiple viewports: call io.AddMouseViewportEvent() with the viewport the OS
    // mouse cursor is hovering. If ImGuiBackendFlags_HasMouseHoveredViewport is not set by the backend, Dear
    // imGui will ignore this field and infer the information using its flawed heuristic.
    // - [!] OSPP backend does NOT correctly ignore viewports with the _NoInputs flag.
    //       Some backend are not able to handle that correctly. If a backend report an hovered viewport that
    //       has the _NoInputs flag (e.g. when dragging a window for docking, the viewport has the _NoInputs
    //       flag in order to allow us to find the viewport under), then Dear ImGui is forced to ignore the
    //       value reported by the backend, and use its flawed heuristic to guess the viewport behind.
    // - [X] OSPP backend correctly reports this regardless of another viewport behind focused and dragged
    // from (we need this to find a useful drag and drop target).
    if(io.BackendFlags & ImGuiBackendFlags_HasMouseHoveredViewport)
    {
        ImGuiID mouse_viewport_id = 0;
        if(ImGuiViewport* mouse_viewport = ImGui_ImplOSPP_GetViewportFromWindowId(bd->MouseWindowID))
        {
            ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(mouse_viewport);
            mouse_viewport_id = mouse_viewport->ID;
        }
        io.AddMouseViewportEvent(mouse_viewport_id);
    }
}

static void ImGui_ImplOSPP_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if(io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if(io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        bd->Window->get_window().show_cursor(false);
    }
    else
    {
        if(bd->LastMouseCursor != imgui_cursor)
        {
            auto cursor = os::get_system_cursor(ImGui_ImplOSPP_MapCursor(imgui_cursor));
            bd->Window->get_window().set_cursor(cursor);

            bd->LastMouseCursor = imgui_cursor;
        }
        bd->Window->get_window().show_cursor(true);
    }
}

static void ImGui_ImplOSPP_UpdateGamepads()
{
}

static void ImGui_ImplOSPP_UpdateMonitors()
{
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();
    bd->WantUpdateMonitors = false;

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    if(!platform_io.Monitors.empty())
    {
        if(!bd->NoMonitorDetected)
        {
            return;
        }
    }

    platform_io.Monitors.resize(0);
    int display_count = os::display::get_available_displays_count();

    if(display_count < 1)
    {
        ImGuiPlatformMonitor monitor;
        monitor.MainPos = monitor.WorkPos = ImVec2();
        monitor.MainSize = monitor.WorkSize = ImVec2(1920.f, 1080.f);
        platform_io.Monitors.push_back(monitor);
        bd->NoMonitorDetected = true;

        return;
    }

    bd->NoMonitorDetected = false;

    for(int n = 0; n < display_count; n++)
    {
        // Warning: the validity of monitor DPI information on Windows depends on the application DPI
        // awareness settings, which generally needs to be set in the manifest or at runtime.
        ImGuiPlatformMonitor monitor;
        auto bounds = os::display::get_bounds(n);
        auto mode = os::display::get_desktop_mode(n);
        monitor.DpiScale = mode.display_scale;
        monitor.MainPos = monitor.WorkPos = ImVec2(float(bounds.x), float(bounds.y));
        monitor.MainSize = monitor.WorkSize = ImVec2(float(bounds.w), float(bounds.h));
        auto usable_bounds = os::display::get_usable_bounds(n);
        monitor.WorkPos = ImVec2((float)usable_bounds.x, (float)usable_bounds.y);
        monitor.WorkSize = ImVec2((float)usable_bounds.w, (float)usable_bounds.h);
        platform_io.Monitors.push_back(monitor);
    }
}

void ImGui_ImplOSPP_NewFrame(float delta_time)
{
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();
    IM_ASSERT(bd != nullptr && "Did you call ImGui_ImplOSPP_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    auto window_size = bd->Window->get_window().get_size();
    io.DisplaySize = ImVec2(float(window_size.w), float(window_size.h));
    io.DeltaTime = delta_time;
    if (io.DeltaTime <= 0.01f)
        io.DeltaTime = 1.0f/60.0f;
    auto window_surface_size = bd->Window->get_surface()->get_size();

    if(window_size.w > 0 && window_size.h > 0)
        io.DisplayFramebufferScale = ImVec2(float(window_surface_size.width) / float(window_size.w),
                                            float(window_surface_size.height) / float(window_size.h));

    // Update monitors
    if(bd->WantUpdateMonitors)
        ImGui_ImplOSPP_UpdateMonitors();

    if(bd->PendingMouseLeaveFrame && bd->PendingMouseLeaveFrame >= ImGui::GetFrameCount() && bd->MouseButtonsDown == 0)
    {
        bd->MouseWindowID = 0;
        bd->PendingMouseLeaveFrame = 0;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    // Our io.AddMouseViewportEvent() calls will only be valid when not capturing.
    // Technically speaking testing for 'bd->MouseButtonsDown == 0' would be more rygorous, but testing for
    // payload reduces noise and potential side-effects.
    if(bd->MouseCanReportHoveredViewport && ImGui::GetDragDropPayload() == nullptr)
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport;
    else
        io.BackendFlags &= ~ImGuiBackendFlags_HasMouseHoveredViewport;

    ImGui_ImplOSPP_UpdateMouseData();
    ImGui_ImplOSPP_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    ImGui_ImplOSPP_UpdateGamepads();
}

void ImGui_ImplOSPP_EndFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to
    // paste this code elsewhere.
    //  For this specific demo app we could also call OSPP_GL_MakeCurrent(window, gl_context) directly)
    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}
//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the backend to create and handle multiple viewports
// simultaneously. If you are new to dear imgui or creating a new binding for dear imgui, it is recommended
// that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

static void ImGui_ImplOSPP_CreateWindow(ImGuiViewport* viewport)
{
    ImGui_ImplOSPP_ViewportData* vd = IM_NEW(ImGui_ImplOSPP_ViewportData)();
    viewport->PlatformUserData = vd;

    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui_ImplOSPP_ViewportData* main_viewport_data = ImGui_ImplOSPP_GetViewportData(main_viewport);

    uint32_t win_flags = 0;
    win_flags |= os::window::hidden;
    win_flags |= (viewport->Flags & ImGuiViewportFlags_NoDecoration) ? os::window::borderless : os::window::resizable;
    win_flags |= (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon) ? os::window::no_taskbar : 0;
    win_flags |= (viewport->Flags & ImGuiViewportFlags_TopMost) ? os::window::always_on_top : 0;

    os::window os_win("No Title Yet",
                      int32_t(viewport->Pos.x),
                      int32_t(viewport->Pos.y),
                      uint32_t(viewport->Size.x),
                      uint32_t(viewport->Size.y),
                      win_flags);

    vd->Window = IM_NEW(ace::render_window)(std::move(os_win));
    vd->WindowOwned = true;

    viewport->PlatformHandle = ImGui_ImplOSPP_IdToHandle(vd->Window->get_window().get_id());
    viewport->PlatformHandleRaw = vd->Window->get_window().get_native_handle();
}

static void ImGui_ImplOSPP_DestroyWindow(ImGuiViewport* viewport)
{
    if(ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport))
    {
        if(vd->Window && vd->WindowOwned)
            IM_DELETE(vd->Window);

        vd->Window = nullptr;
        IM_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void ImGui_ImplOSPP_ShowWindow(ImGuiViewport* viewport)
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);

    vd->Window->get_window().show();
}

static auto ImGui_ImplOSPP_GetWindowPos(ImGuiViewport* viewport) -> ImVec2
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    auto pos = vd->Window->get_window().get_position();
    return {float(pos.x), float(pos.y)};
}

static void ImGui_ImplOSPP_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    vd->Window->get_window().set_position(int32_t(pos.x), int32_t(pos.y));
}

static auto ImGui_ImplOSPP_GetWindowSize(ImGuiViewport* viewport) -> ImVec2
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    auto size = vd->Window->get_window().get_size();
    return {float(size.w), float(size.h)};
}

static void ImGui_ImplOSPP_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    vd->Window->resize(uint32_t(size.x), uint32_t(size.y));
}

static void ImGui_ImplOSPP_SetWindowTitle(ImGuiViewport* viewport, const char* title)
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    vd->Window->get_window().set_title(title);
}

static void ImGui_ImplOSPP_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    vd->Window->get_window().set_opacity(alpha);
}

static void ImGui_ImplOSPP_SetWindowFocus(ImGuiViewport* viewport)
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    vd->Window->get_window().request_focus();
}

static auto ImGui_ImplOSPP_GetWindowFocus(ImGuiViewport* viewport) -> bool
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    return vd->Window->get_window().has_focus();
}

static auto ImGui_ImplOSPP_GetWindowMinimized(ImGuiViewport* viewport) -> bool
{
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);
    return vd->Window->get_window().is_minimized();
}

void ImGui_ImplOSPP_RenderWindow(ImGuiViewport* viewport, void* rend_args)
{
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);

    bd->RenderCallback(vd->Window, viewport, rend_args);
}

void ImGui_ImplOSPP_SwapBuffers(ImGuiViewport* viewport, void* rend_args)
{
    ImGui_ImplOSPP_Data* bd = ImGui_ImplOSPP_GetBackendData();
    ImGui_ImplOSPP_ViewportData* vd = ImGui_ImplOSPP_GetViewportData(viewport);

    bd->SwapCallback(vd->Window, viewport, rend_args);
}

static void ImGui_ImplOSPP_InitPlatformInterface(ace::render_window* window)
{
    // Register platform interface (will be coupled with a renderer interface)
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_CreateWindow = ImGui_ImplOSPP_CreateWindow;
    platform_io.Platform_DestroyWindow = ImGui_ImplOSPP_DestroyWindow;
    platform_io.Platform_ShowWindow = ImGui_ImplOSPP_ShowWindow;
    platform_io.Platform_SetWindowPos = ImGui_ImplOSPP_SetWindowPos;
    platform_io.Platform_GetWindowPos = ImGui_ImplOSPP_GetWindowPos;
    platform_io.Platform_SetWindowSize = ImGui_ImplOSPP_SetWindowSize;
    platform_io.Platform_GetWindowSize = ImGui_ImplOSPP_GetWindowSize;
    platform_io.Platform_SetWindowFocus = ImGui_ImplOSPP_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = ImGui_ImplOSPP_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = ImGui_ImplOSPP_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = ImGui_ImplOSPP_SetWindowTitle;
    platform_io.Platform_RenderWindow = ImGui_ImplOSPP_RenderWindow;
    platform_io.Platform_SwapBuffers = ImGui_ImplOSPP_SwapBuffers;
    platform_io.Platform_SetWindowAlpha = ImGui_ImplOSPP_SetWindowAlpha;

    // Register main window handle (which is owned by the main application, not by us)
    // This is mostly for simplicity and consistency, so that our code (e.g. mouse handling etc.) can use same
    // logic for main and secondary viewports.
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui_ImplOSPP_ViewportData* vd = IM_NEW(ImGui_ImplOSPP_ViewportData)();
    vd->Window = window;
    vd->WindowOwned = false;

    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = ImGui_ImplOSPP_IdToHandle(window->get_window().get_id());
    main_viewport->PlatformHandleRaw = window->get_window().get_native_handle();
}

static void ImGui_ImplOSPP_ShutdownPlatformInterface()
{
    ImGui::DestroyPlatformWindows();
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
