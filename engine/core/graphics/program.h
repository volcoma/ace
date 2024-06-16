#pragma once

#include "frame_buffer.h"
#include "shader.h"
#include "texture.h"
#include "uniform.h"

#include "handle_impl.h"
#include <hpp/string_view.hpp>
#include <limits>
#include <map>
#include <memory>

namespace gfx
{

struct program : public handle_impl<program, program_handle>
{
    using uniform_ptr = std::shared_ptr<uniform>;
    //-----------------------------------------------------------------------------
    //  Name : program ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    program() = default;

    //-----------------------------------------------------------------------------
    //  Name : program ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    program(const shader& compute_shader);

    //-----------------------------------------------------------------------------
    //  Name : program ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    program(const shader& vertex_shader, const shader& fragment_shader);

    //-----------------------------------------------------------------------------
    //  Name : set_texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_texture(uint8_t _stage,
                     const hpp::string_view& _sampler,
                     const gfx::frame_buffer* _handle,
                     uint8_t _attachment = 0,
                     uint32_t _flags = std::numeric_limits<uint32_t>::max());

    //-----------------------------------------------------------------------------
    //  Name : set_texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_texture(uint8_t _stage,
                     const hpp::string_view& _sampler,
                     const gfx::texture* _texture,
                     uint32_t _flags = std::numeric_limits<uint32_t>::max());

    //-----------------------------------------------------------------------------
    //  Name : set_uniform ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_uniform(const hpp::string_view& _name, const void* _value, uint16_t _num = 1);

    //-----------------------------------------------------------------------------
    //  Name : get_uniform ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_uniform(const hpp::string_view& _name, uint8_t stage = uint8_t(-1)) -> uniform*;

    /// All uniforms for this program.
    std::map<std::string, uniform_ptr, std::less<>> uniforms;

    std::array<uniform*, 64> textures_uniforms;
};
} // namespace gfx
