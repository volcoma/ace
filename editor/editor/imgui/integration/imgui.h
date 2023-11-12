/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef IMGUI_H_HEADER_GUARD
#define IMGUI_H_HEADER_GUARD

#include <engine/rendering/render_window.h>

#include "fonts/icons/icons_font_awesome.h"
#include "fonts/icons/icons_kenney.h"
#include <imgui_includes.h>

#include <engine/assets/asset_handle.h>
#include <graphics/texture.h>

inline uint32_t imguiRGBA(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255)
{
    return 0 | (uint32_t(_r) << 0) | (uint32_t(_g) << 8) | (uint32_t(_b) << 16) | (uint32_t(_a) << 24);
}

namespace bx
{
struct AllocatorI;
}

void imguiCreate(render_window* window, float _fontSize = 18.0f, bx::AllocatorI* _allocator = NULL);
void imguiDestroy();

void imguiProcessEvent(const os::event& e);

void imguiBeginFrame(float dt);
void imguiEndFrame(gfx::view_id id);

namespace ImGui
{
#define IMGUI_FLAGS_NONE        UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

struct Font
{
    enum Enum
    {
        Thin,
        ExtraLight,
        Light,
        Regular,
        Medium,
        SemiBold,
        Bold,
        ExtraBold,
        Black,

        Mono,
        Count
    };
};

void PushFont(Font::Enum _font);

void PushWindowFontSize(int size);
void PopWindowFontSize();
///
inline ImTextureID ToId(gfx::texture_handle _handle, uint8_t _flags, uint8_t _mip)
{
    union
    {
        struct
        {
            gfx::texture_handle handle;
            uint8_t flags;
            uint8_t mip;
        } s;
        ImTextureID id;
    } tex;
    tex.s.handle = _handle;
    tex.s.flags = _flags;
    tex.s.mip = _mip;
    return tex.id;
}

inline ImTextureID ToId(const asset_handle<gfx::texture>& _handle, uint8_t _flags = IMGUI_FLAGS_ALPHA_BLEND, uint8_t _mip = 0)
{
    return ToId(_handle.get().native_handle(), _flags, _mip);
}

inline ImVec2 GetSize(const asset_handle<gfx::texture>& _handle, const ImVec2& fallback = {})
{
    if(_handle.is_ready())
    {
        const auto& tex = _handle.get();
        return ImVec2{float(tex.info.width), float(tex.info.height)};
    }

    return fallback;
}

// Helper function for passing gfx::texture_handle to ImGui::Image.
inline void Image(gfx::texture_handle _handle,
                  uint8_t _flags,
                  uint8_t _mip,
                  const ImVec2& _size,
                  const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                  const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                  const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                  const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
{
    Image(ToId(_handle, _flags, _mip), _size, _uv0, _uv1, _tintCol, _borderCol);
}

// Helper function for passing gfx::texture_handle to ImGui::Image.
inline void Image(gfx::texture_handle _handle,
                  const ImVec2& _size,
                  const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                  const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                  const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                  const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
{
    Image(_handle, IMGUI_FLAGS_ALPHA_BLEND, 0, _size, _uv0, _uv1, _tintCol, _borderCol);
}

// Helper function for passing gfx::texture_handle to ImGui::ImageButton.
inline bool ImageButton(gfx::texture_handle _handle,
                        uint8_t _flags,
                        uint8_t _mip,
                        const ImVec2& _size,
                        const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                        const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                        const ImVec4& _bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
                        const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
{
    return ImageButton("image", ToId(_handle, _flags, _mip), _size, _uv0, _uv1, _bgCol, _tintCol);
}

// Helper function for passing gfx::texture_handle to ImGui::ImageButton.
inline bool ImageButton(gfx::texture_handle _handle,
                        const ImVec2& _size,
                        const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                        const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                        const ImVec4& _bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
                        const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
{
    return ImageButton(_handle, IMGUI_FLAGS_ALPHA_BLEND, 0, _size, _uv0, _uv1, _bgCol, _tintCol);
}

///
inline void NextLine()
{
    SetCursorPosY(GetCursorPosY() + GetTextLineHeightWithSpacing());
}

///
inline bool MouseOverArea()
{
    return false || ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered() ||
           ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGuizmo::IsOver();
}

///
void PushEnabled(bool _enabled);

///
void PopEnabled();

template<size_t BufferSize = 64>
inline auto CreateInputTextBuffer(const std::string& name) -> std::array<char, BufferSize>
{
    std::array<char, BufferSize> input_buff;
    input_buff.fill(0);
    auto name_sz = std::min(name.size(), input_buff.size() - 1);
    std::memcpy(input_buff.data(), name.c_str(), name_sz);
    return input_buff;
}

} // namespace ImGui

#endif // IMGUI_H_HEADER_GUARD
