#include "reflection_probe_component.h"

namespace ace
{

auto reflection_probe_component::get_bounds() const -> math::bbox
{
    if(probe_.type == probe_type::sphere)
    {
        auto sphere = math::bsphere(math::vec3(0.0f, 0.0f, 0.0f), probe_.sphere_data.range);
        math::bbox result;
        result.from_sphere(sphere.position, sphere.radius);
        return result;
    }
    else if(probe_.type == probe_type::box)
    {
        math::bbox result;
        result.min = -probe_.box_data.extents;
        result.max = probe_.box_data.extents;
        return result;
    }

    return {};
}

auto reflection_probe_component::compute_projected_sphere_rect(irect32_t& rect,
                                                               const math::vec3& position,
                                                               const math::vec3& scale,
                                                               const math::vec3& view_origin,
                                                               const math::transform& view,
                                                               const math::transform& proj) const -> int
{
    if(probe_.type == probe_type::sphere)
    {
        return math::compute_projected_sphere_rect(rect.left,
                                                   rect.right,
                                                   rect.top,
                                                   rect.bottom,
                                                   position,
                                                   probe_.sphere_data.range * math::max(scale.x, math::max(scale.y, scale.z)),
                                                   view_origin,
                                                   view,
                                                   proj);
    }
    else if(probe_.type == probe_type::box)
    {
        float w2 = math::pow(scale.x * probe_.box_data.extents.x * 2.0f, 2.0f);
        float h2 = math::pow(scale.y * probe_.box_data.extents.y * 2.0f, 2.0f);
        float l2 = math::pow(scale.z * probe_.box_data.extents.z * 2.0f, 2.0f);
        float d2 = w2 + h2 + l2;
        float d = math::sqrt(d2);

        return math::compute_projected_sphere_rect(rect.left,
                                                   rect.right,
                                                   rect.top,
                                                   rect.bottom,
                                                   position,
                                                   d,
                                                   view_origin,
                                                   view,
                                                   proj);
    }
    else
    {
        return 1;
    }
}

auto reflection_probe_component::get_render_view(size_t idx) -> gfx::render_view&
{
    return rview_[idx];
}

auto reflection_probe_component::get_cubemap() -> const  gfx::texture::ptr&
{
    auto& tex = rview_[0].tex_get_or_emplace("CUBEMAP");
    if(!tex)
    {
        constexpr uint16_t size = 256;
        tex = std::make_shared<gfx::texture>(size,
                                             true,
                                             1,
                                             gfx::texture_format::RGBA8S,
                                             BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_RT);
    }

    return tex;
}

auto reflection_probe_component::get_cubemap_fbo(size_t face) -> const gfx::frame_buffer::ptr&
{
    auto& fbo = rview_[face].fbo_get_or_emplace("CUBEMAP");
    if(!fbo)
    {
        gfx::fbo_attachment att;
        att.layer = face;
        att.texture = get_cubemap();

        fbo = std::make_shared<gfx::frame_buffer>();
        fbo->populate({att});
    }


    return fbo;
}

void reflection_probe_component::update()
{

    // Check if all faces have been generated; if so, reset the state
    bool fully_generated = true;
    for(auto& frame : generated_frame_)
    {
        fully_generated &= frame != uint64_t(-1);
    }


    if(fully_generated)
    {
        for(auto& frame : generated_frame_)
        {
            frame = uint64_t(-1); // Reset to an initial invalid state
        }
    }

    if(fully_generated)
    {
        first_generation_ = false;
    }
    generated_faces_count_ = 0; // Reset the count of generated faces
}

auto reflection_probe_component::get_probe() const -> const reflection_probe&
{
    return probe_;
}

void reflection_probe_component::set_probe(const reflection_probe& probe)
{
    if(probe == probe_)
    {
        return;
    }

    touch();

    probe_ = probe;
}

auto reflection_probe_component::already_generated() const -> bool
{
    bool generated = true;
    for(size_t i = 0; i < generated_frame_.size(); ++i)
    {
        generated &= already_generated(i);
    }
    // Check if all faces have been generated in the current cycle
    return generated;
}

auto reflection_probe_component::already_generated(size_t face) const -> bool
{
    if(!first_generation_)
    {
        if(generated_faces_count_ == faces_per_frame_)
        {
            return true;
        }
    }

    // Return true if the face has been generated in the current cycle
    return generated_frame_[face] != uint64_t(-1);
}
void reflection_probe_component::set_generation_frame(size_t face, uint64_t frame)
{
    generated_frame_[face] = frame;
    generated_faces_count_++;
}
} // namespace ace
