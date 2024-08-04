#pragma once
#include "frame_buffer.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace gfx
{
struct render_pass
{
    static void push_scope(const char* name);
    static void pop_scope();

    //-----------------------------------------------------------------------------
    //  Name : render_pass ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    render_pass(const char* name);
    render_pass(view_id id, const char* name);


    //-----------------------------------------------------------------------------
    //  Name : bind ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void bind(const frame_buffer* fb = nullptr) const;
    void touch() const;
    //-----------------------------------------------------------------------------
    //  Name : clear ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void clear(uint16_t _flags, uint32_t _rgba = 0x000000ff, float _depth = 1.0f, uint8_t _stencil = 0) const;

    //-----------------------------------------------------------------------------
    //  Name : clear ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void clear() const;

    //-----------------------------------------------------------------------------
    //  Name : set_view_proj ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_view_proj(const float* v, const float* p);

    //-----------------------------------------------------------------------------
    //  Name : reset ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    static void reset();

    //-----------------------------------------------------------------------------
    //  Name : get_pass ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    static auto get_max_pass_id() -> gfx::view_id;

    static auto get_last_frame_max_pass_id() -> gfx::view_id;
    ///
    gfx::view_id id;
};
} // namespace gfx
