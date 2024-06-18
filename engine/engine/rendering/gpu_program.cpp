#include "gpu_program.h"
#include <algorithm>

namespace ace
{
gpu_program::gpu_program(asset_handle<gfx::shader> compute_shader)
{
    attach_shader(compute_shader);
    populate();
}

gpu_program::gpu_program(asset_handle<gfx::shader> vertex_shader, asset_handle<gfx::shader> fragment_shader)
{
    attach_shader(vertex_shader);
    attach_shader(fragment_shader);
    populate();
}

void gpu_program::attach_shader(asset_handle<gfx::shader> shader)
{
    if(!shader)
    {
        shaders_cached_.push_back(gfx::shader::invalid_handle().idx);
        shaders_.push_back(shader);
        return;
    }
    
    shaders_cached_.push_back(shader.get()->native_handle().idx);
    shaders_.push_back(shader);
}

void gpu_program::populate()
{
    bool all_valid = std::all_of(std::begin(shaders_),
                                 std::end(shaders_),
                                 [](auto& shader)
                                 {
                                     return shader && shader.get()->is_valid();
                                 });

    if(all_valid)
    {
        if(shaders_.size() == 1)
        {
            const auto& compute_shader = shaders_[0];
            program_ = std::make_shared<gfx::program>(*compute_shader.get());
        }
        else if(shaders_.size() == 2)
        {
            const auto& vertex_shader = shaders_[0];
            const auto& fragment_shader = shaders_[1];
            program_ = std::make_shared<gfx::program>(*vertex_shader.get(), *fragment_shader.get());
        }

        shaders_cached_.clear();
        for(const auto& shader : shaders_)
        {
            shaders_cached_.push_back(shader.get()->native_handle().idx);
        }
    }
}

void gpu_program::set_texture(uint8_t _stage,
                              const hpp::string_view& _sampler,
                              const gfx::frame_buffer* _fbo,
                              uint8_t _attachment,
                              uint32_t _flags)
{
    program_->set_texture(_stage, _sampler, _fbo, _attachment, _flags);
}

void gpu_program::set_texture(uint8_t _stage,
                              const hpp::string_view& _sampler,
                              const gfx::texture* _texture,
                              uint32_t _flags)
{
    program_->set_texture(_stage, _sampler, _texture, _flags);
}

void gpu_program::set_uniform(const hpp::string_view& _name, const void* _value, uint16_t _num)
{
    program_->set_uniform(_name, _value, _num);
}

void gpu_program::set_uniform(const hpp::string_view& _name, const math::vec4& _value, uint16_t _num)
{
    set_uniform(_name, math::value_ptr(_value), _num);
}

void gpu_program::set_uniform(const hpp::string_view& _name, const math::vec3& _value, uint16_t _num)
{
    set_uniform(_name, math::vec4(_value, 0.0f), _num);
}

void gpu_program::set_uniform(const hpp::string_view& _name, const math::vec2& _value, uint16_t _num)
{
    set_uniform(_name, math::vec4(_value, 0.0f, 0.0f), _num);
}

gfx::program::uniform_ptr gpu_program::get_uniform(const hpp::string_view& _name)
{
    return program_->get_uniform(_name);
}

gfx::program::handle_type_t gpu_program::native_handle() const
{
    return program_->native_handle();
}

const std::vector<asset_handle<gfx::shader>>& gpu_program::get_shaders() const
{
    return shaders_;
}

bool gpu_program::is_valid() const
{
    return program_ && program_->is_valid();
}

bool gpu_program::begin()
{
    bool repopulate = false;
    for(std::size_t i = 0; i < shaders_cached_.size(); ++i)
    {
        auto shader_ptr = shaders_[i];
        if(!shader_ptr)
            continue;
        
        if(shaders_cached_[i] != shader_ptr.get()->native_handle().idx)
        {
            repopulate = true;
            break;
        }
    }

    if(repopulate)
        populate();

    return is_valid();
}

void gpu_program::end()
{
    //    gfx::discard();
}


} // namespace ace


namespace gfx
{

void set_texture(const gfx::program::uniform_ptr& uniform,
                 std::uint8_t _stage,
                 const gfx::frame_buffer* _handle,
                 uint8_t _attachment,
                 std::uint32_t _flags)
{
    if(!uniform)
    {
        return;
    }

    uniform->set_texture(_stage, _handle, _attachment, _flags);
}

void set_texture(const gfx::program::uniform_ptr& uniform,
                 std::uint8_t _stage,
                 const gfx::texture* _texture,
                 std::uint32_t _flags)
{
    if(!uniform)
    {
        return;
    }

    uniform->set_texture(_stage, _texture, _flags);
}


void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const void* _value, std::uint16_t _num)
{
    if(!uniform)
    {
        return;
    }

    uniform->set_uniform(_value, _num);
}
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::mat4& _value, std::uint16_t _num)
{
    set_uniform(uniform, math::value_ptr(_value), _num);
}
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::vec4& _value, std::uint16_t _num)
{
    set_uniform(uniform, math::value_ptr(_value), _num);

}
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::vec3& _value, std::uint16_t _num)
{
    set_uniform(uniform, math::value_ptr(_value), _num);

}
void set_uniform(const gfx::program::uniform_ptr& uniform,
                 const math::vec2& _value, std::uint16_t _num)
{
    set_uniform(uniform, math::value_ptr(_value), _num);

}
}
