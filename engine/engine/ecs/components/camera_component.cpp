#include "camera_component.h"

namespace ace
{
camera_component::camera_component()
{
    camera_.set_viewport_size({640, 480});
}

void camera_component::update(const math::transform& t)
{
    // Release the unused fbos and textures
    render_view_.release_unused_resources();
    // First update so the camera can cache the previous matrices
    camera_.record_current_matrices();
    // Set new transform
    camera_.look_at(t.get_position(), t.get_position() + t.z_unit_axis(), t.y_unit_axis());
}

auto camera_component::get_hdr() const -> bool
{
    return hdr_;
}

void camera_component::set_hdr(bool hdr)
{
    hdr_ = hdr;
}

void camera_component::set_viewport_size(const usize32_t& size)
{
    camera_.set_viewport_size(size);
}

auto camera_component::get_viewport_size() const -> const usize32_t&
{
    return camera_.get_viewport_size();
}

auto camera_component::get_ortho_size() const -> float
{
    return camera_.get_ortho_size();
}

void camera_component::set_ortho_size(float size)
{
    camera_.set_orthographic_size(size);
}

auto camera_component::get_ppu() const -> float
{
    return camera_.get_ppu();
}

auto camera_component::get_render_view() -> gfx::render_view&
{
    return render_view_;
}

void camera_component::set_fov(float fovDegrees)
{
    camera_.set_fov(fovDegrees);
}

void camera_component::set_near_clip(float distance)
{
    camera_.set_near_clip(distance);
}

void camera_component::set_far_clip(float distance)
{
    camera_.set_far_clip(distance);
}

void camera_component::set_projection_mode(projection_mode mode)
{
    camera_.set_projection_mode(mode);
}

auto camera_component::get_fov() const -> float
{
    return camera_.get_fov();
}
auto camera_component::get_near_clip() const -> float
{
    return camera_.get_near_clip();
}
auto camera_component::get_far_clip() const -> float
{
    return camera_.get_far_clip();
}

auto camera_component::get_projection_mode() const -> projection_mode
{
    return camera_.get_projection_mode();
}

auto camera_component::get_camera() -> camera&
{
    return camera_;
}

auto camera_component::get_camera() const -> const camera&
{
    return camera_;
}
} // namespace ace
