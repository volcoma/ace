#pragma once

#include <engine/assets/asset_handle.h>

#include <graphics/program.h>
#include <graphics/shader.h>
#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

namespace ace
{
class gpu_program
{
public:
    using ptr = std::shared_ptr<gpu_program>;
    using wptr = std::weak_ptr<gpu_program>;
    using uptr = std::unique_ptr<gpu_program>;

    REFLECTABLE(gpu_program)
    SERIALIZABLE(gpu_program)

    gpu_program() = default;
    //-----------------------------------------------------------------------------
    //  Name : gpu_program (Constructor)
    /// <summary>
    /// Creates a program form a compute shader asset.
    /// </summary>
    gpu_program(asset_handle<gfx::shader> compute_shader);

    //-----------------------------------------------------------------------------
    //  Name : gpu_program (Constructor)
    /// <summary>
    /// Creates a program form a vertex and fragment shader assets.
    /// </summary>
    //-----------------------------------------------------------------------------
    gpu_program(asset_handle<gfx::shader> vertex_shader, asset_handle<gfx::shader> fragment_shader);

    //-----------------------------------------------------------------------------
    //  Name : begin ()
    /// <summary>
    /// Begins usage of the program. Checks validity of attached shaders and
    /// recreates the internal program if necessary.
    /// </summary>
    //-----------------------------------------------------------------------------
    bool begin();

    //-----------------------------------------------------------------------------
    //  Name : end ()
    /// <summary>
    /// Indicates end of working with a program.
    /// </summary>
    //-----------------------------------------------------------------------------
    void end();

    //-----------------------------------------------------------------------------
    //  Name : set_texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_texture(std::uint8_t _stage,
                     const hpp::string_view& _sampler,
                     const gfx::frame_buffer* _handle,
                     uint8_t _attachment = 0,
                     std::uint32_t _flags = std::numeric_limits<std::uint32_t>::max());

    //-----------------------------------------------------------------------------
    //  Name : set_texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_texture(std::uint8_t _stage,
                     const hpp::string_view& _sampler,
                     const gfx::texture* _texture,
                     std::uint32_t _flags = std::numeric_limits<std::uint32_t>::max());

    //-----------------------------------------------------------------------------
    //  Name : set_uniform ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_uniform(const hpp::string_view& _name, const void* _value, std::uint16_t _num = 1);
    void set_uniform(const hpp::string_view& _name, const math::vec4& _value, std::uint16_t _num = 1);
    void set_uniform(const hpp::string_view& _name, const math::vec3& _value, std::uint16_t _num = 1);
    void set_uniform(const hpp::string_view& _name, const math::vec2& _value, std::uint16_t _num = 1);

    //-----------------------------------------------------------------------------
    //  Name : get_uniform ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    gfx::program::uniform_ptr get_uniform(const hpp::string_view& _name);

    //-----------------------------------------------------------------------------
    //  Name : native_handle ()
    /// <summary>
    /// Retrieves the native handle of the internal shader program.
    /// </summary>
    //-----------------------------------------------------------------------------
    gfx::program::handle_type_t native_handle() const;

    //-----------------------------------------------------------------------------
    //  Name : get_shaders ()
    /// <summary>
    /// Retrieves the shader assets that created the shader program.
    /// </summary>
    //-----------------------------------------------------------------------------
    const std::vector<asset_handle<gfx::shader>>& get_shaders() const;

    bool is_valid() const;

private:
    void populate();
    void attach_shader(asset_handle<gfx::shader> shader);

    /// Shaders that created this program.
    std::vector<asset_handle<gfx::shader>> shaders_;
    /// Shaders that created this program.
    std::vector<std::uint16_t> shaders_cached_;
    /// program
    std::shared_ptr<gfx::program> program_;
};

struct uniforms_cache
{
    void cache_uniform(gpu_program* program, gfx::program::uniform_ptr& uniform, const hpp::string_view& name)
    {
        uniform = program->get_uniform(name);
    }
};

} // namespace ace

namespace gfx
{

void set_transform(const std::vector<math::transform::mat4_t>& matrices);
void set_transform(const std::vector<math::transform>& matrices);

void set_transform(const math::transform::mat4_t& matrix);
void set_transform(const math::transform& matrix);


void set_texture(const gfx::program::uniform_ptr& uniform,
                 std::uint8_t _stage,
                 const gfx::frame_buffer* _handle,
                 uint8_t _attachment = 0,
                 std::uint32_t _flags = std::numeric_limits<std::uint32_t>::max());

void set_texture(const gfx::program::uniform_ptr& uniform,
                 std::uint8_t _stage,
                 const gfx::texture* _texture,
                 std::uint32_t _flags = std::numeric_limits<std::uint32_t>::max());

void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const void* _value, std::uint16_t _num = 1);
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::mat4& _value, std::uint16_t _num = 1);
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::vec4& _value, std::uint16_t _num = 1);
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::vec3& _value, std::uint16_t _num = 1);
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::vec2& _value, std::uint16_t _num = 1);
}




