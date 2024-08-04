#include "camera_component.h"

namespace ace
{
camera_component::camera_component()
{
    pipeline_camera_.get_camera().set_viewport_size({640, 480});
}

void camera_component::update(const math::transform& t)
{
    // Release the unused fbos and textures
    render_view_.release_unused_resources();

    pipeline_camera_.get_camera().record_current_matrices();
    pipeline_camera_.get_camera().look_at(t.get_position(), t.get_position() + t.z_unit_axis(), t.y_unit_axis());
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
    pipeline_camera_.get_camera().set_viewport_size(size);
}

auto camera_component::get_viewport_size() const -> const usize32_t&
{
    return pipeline_camera_.get_camera().get_viewport_size();
}

auto camera_component::get_ortho_size() const -> float
{
    return pipeline_camera_.get_camera().get_ortho_size();
}

void camera_component::set_ortho_size(float size)
{
    pipeline_camera_.get_camera().set_orthographic_size(size);
}

auto camera_component::get_ppu() const -> float
{
    return pipeline_camera_.get_camera().get_ppu();
}

auto camera_component::get_render_view() -> gfx::render_view&
{
    return render_view_;
}

auto camera_component::get_storage() -> camera_storage&
{
    return storage_;
}

auto camera_component::get_pipeline_data() -> pipeline_camera&
{
    return pipeline_camera_;
}

auto camera_component::get_pipeline_data() const -> const pipeline_camera&
{
    return pipeline_camera_;
}

void camera_component::set_fov(float fovDegrees)
{
    pipeline_camera_.get_camera().set_fov(fovDegrees);
}

void camera_component::set_near_clip(float distance)
{
    pipeline_camera_.get_camera().set_near_clip(distance);
}

void camera_component::set_far_clip(float distance)
{
    pipeline_camera_.get_camera().set_far_clip(distance);
}

void camera_component::set_projection_mode(projection_mode mode)
{
    pipeline_camera_.get_camera().set_projection_mode(mode);
}

auto camera_component::get_fov() const -> float
{
    return pipeline_camera_.get_camera().get_fov();
}
auto camera_component::get_near_clip() const -> float
{
    return pipeline_camera_.get_camera().get_near_clip();
}
auto camera_component::get_far_clip() const -> float
{
    return pipeline_camera_.get_camera().get_far_clip();
}

auto camera_component::get_projection_mode() const -> projection_mode
{
    return pipeline_camera_.get_camera().get_projection_mode();
}

auto camera_component::get_camera() -> camera&
{
    return pipeline_camera_.get_camera();
}

auto camera_component::get_camera() const -> const camera&
{
    return pipeline_camera_.get_camera();
}
} // namespace ace
