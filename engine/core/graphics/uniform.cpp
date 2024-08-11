#include "uniform.h"

namespace gfx
{

uniform::uniform(const std::string& _name, uniform_type _type, std::uint16_t _num /*= 1*/)
{
    handle_ = gfx::create_uniform(_name.c_str(), _type, _num);
    gfx::get_uniform_info(handle_, info);
}

uniform::uniform(handle_type_t _handle)
{
    gfx::get_uniform_info(_handle, info);
    handle_ = gfx::create_uniform(info.name, info.type, info.num);
}


void uniform::set_texture(uint8_t _stage,
                          const gfx::frame_buffer* frameBuffer,
                          uint8_t _attachment /*= 0 */,
                          uint32_t _flags /*= std::numeric_limits<uint32_t>::max()*/)
{
    if(frameBuffer == nullptr)
    {
        return;
    }

    gfx::set_texture(_stage, native_handle(), frameBuffer->get_texture(_attachment)->native_handle(), _flags);
}

void uniform::set_texture(uint8_t _stage,
                          const gfx::texture* _texture,
                          uint32_t _flags /*= std::numeric_limits<uint32_t>::max()*/)
{
    if(_texture == nullptr)
    {
        return;
    }

    gfx::set_texture(_stage, native_handle(), _texture->native_handle(), _flags);
}

void uniform::set_uniform(const void* _value, uint16_t _num)
{
    gfx::set_uniform(native_handle(), _value, _num);
}
} // namespace gfx
