/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef IMGUI_H_HEADER_GUARD
#define IMGUI_H_HEADER_GUARD

#include <engine/rendering/render_window.h>

#include "fonts/icons/icons_material_design_icons.h"

#include "graphics/graphics.h"
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

void imguiCreate(ace::render_window* window, float _fontSize = 18.0f, bx::AllocatorI* _allocator = NULL);
void imguiDestroy();

void imguiProcessEvent(const os::event& e);

void imguiBeginFrame(float dt);
void imguiEndFrame(gfx::view_id id);

namespace ImGui
{
#define IMGUI_FLAGS_NONE        UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)
#define IMGUI_FLAGS_FLIP_UV     UINT8_C(0x02)

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
        BigIcons,
        Count
    };
};

uint64_t GetDrawCalls();

void PushFont(Font::Enum _font);
ImFont* GetFont(Font::Enum _font);
void PushWindowFontSize(int size);
void PopWindowFontSize();
void PushWindowFontScale(float scale);
void PopWindowFontScale();

void KeepAliveOneFrame(const gfx::texture::ptr& tex);

union ImTexture
{
    struct
    {
        gfx::texture_handle handle;
        gfx::program_handle phandle;
        uint8_t flags;
        uint8_t mip;
        uint8_t index;
    } s;
    ImTextureID id;
};


inline ImTexture ToTex(gfx::texture_handle _handle, uint8_t _index, gfx::program_handle _phandle, uint8_t _mip = 0, uint8_t _flags = IMGUI_FLAGS_ALPHA_BLEND)
{
    ImTexture tex;
    tex.s.handle = _handle;
    tex.s.phandle = _phandle;
    tex.s.flags = _flags;
    tex.s.mip = _mip;
    tex.s.index = _index;
    return tex;
}
///
inline ImTextureID ToId(gfx::texture_handle _handle, uint8_t _mip = 0, uint8_t _flags = IMGUI_FLAGS_ALPHA_BLEND)
{
    ImTexture tex = ToTex(_handle, 0, {gfx::invalid_handle}, _mip, _flags);
    return tex.id;
}

inline ImTextureID ToId(const gfx::texture& _handle, uint8_t _mip = 0, uint8_t _flags = IMGUI_FLAGS_ALPHA_BLEND)
{
    if(_handle.is_valid() && _handle.is_render_target() && gfx::is_origin_bottom_left())
    {
        _flags |= IMGUI_FLAGS_FLIP_UV;
    }

    return ToId(_handle.native_handle(), _mip, _flags);
}


inline ImTextureID ToId(const gfx::texture::ptr& _handle, uint8_t _mip = 0, uint8_t _flags = IMGUI_FLAGS_ALPHA_BLEND)
{
    if(!_handle)
    {
        return nullptr;
    }

    KeepAliveOneFrame(_handle);
    return ToId(*_handle, _mip, _flags);
}

inline ImTextureID ToId(const asset_handle<gfx::texture>& _handle, uint8_t _mip = 0, uint8_t _flags = IMGUI_FLAGS_ALPHA_BLEND)
{
    return ToId(_handle.get(), _mip, _flags);
}

inline ImVec2 GetSize(const gfx::texture& tex, const ImVec2& fallback = {})
{
    return ImVec2{float(tex.info.width), float(tex.info.height)};
}

inline ImVec2 GetSize(const gfx::texture::ptr& tex, const ImVec2& fallback = {})
{
    if(tex)
    {
        return GetSize(*tex, fallback);
    }

    return fallback;
}

inline ImVec2 GetSize(const asset_handle<gfx::texture>& _handle, const ImVec2& fallback = {})
{
    if(_handle.is_ready())
    {
        return GetSize(_handle.get(), fallback);
    }

    return fallback;
}

// Helper function for passing gfx::texture_handle to ImGui::Image.
inline void Image(gfx::texture_handle _handle,
                  uint8_t _mip,
                  uint8_t _flags,
                  const ImVec2& _size,
                  const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                  const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                  const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                  const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
{
    Image(ToId(_handle, _mip, _flags), _size, _uv0, _uv1, _tintCol, _borderCol);
}

// Helper function for passing gfx::texture_handle to ImGui::Image.
inline void Image(gfx::texture_handle _handle,
                  const ImVec2& _size,
                  const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                  const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                  const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                  const ImVec4& _borderCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f))
{
    Image(_handle, 0, IMGUI_FLAGS_ALPHA_BLEND, _size, _uv0, _uv1, _tintCol, _borderCol);
}

// Helper function for passing gfx::texture_handle to ImGui::ImageButton.
inline bool ImageButton(gfx::texture_handle _handle,
                        uint8_t _mip,
                        uint8_t _flags,
                        const ImVec2& _size,
                        const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                        const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                        const ImVec4& _bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
                        const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
{
    return ImageButton("image", ToId(_handle, _mip, _flags), _size, _uv0, _uv1, _bgCol, _tintCol);
}

// Helper function for passing gfx::texture_handle to ImGui::ImageButton.
inline bool ImageButton(gfx::texture_handle _handle,
                        const ImVec2& _size,
                        const ImVec2& _uv0 = ImVec2(0.0f, 0.0f),
                        const ImVec2& _uv1 = ImVec2(1.0f, 1.0f),
                        const ImVec4& _bgCol = ImVec4(0.0f, 0.0f, 0.0f, 0.0f),
                        const ImVec4& _tintCol = ImVec4(1.0f, 1.0f, 1.0f, 1.0f))
{
    return ImageButton(_handle, 0, IMGUI_FLAGS_ALPHA_BLEND, _size, _uv0, _uv1, _bgCol, _tintCol);
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

bool IsReadonly();
///
void PushReadonly(bool _enabled);

///
void PopReadonly();
///
void PushEnabled(bool _enabled);

///
void PopEnabled();

} // namespace ImGui

#endif // IMGUI_H_HEADER_GUARD
