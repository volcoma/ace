#include "camera_component.hpp"

#include <engine/meta/rendering/camera.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(camera_component)
{
    rttr::registration::class_<camera_component>("camera_component")(rttr::metadata("category", "RENDERING"),
                                                                     rttr::metadata("pretty_name", "Camera"))
        .constructor<>()
        .property("projection_mode", &camera_component::get_projection_mode, &camera_component::set_projection_mode)(
            rttr::metadata("pretty_name", "Projection Mode"))
        .property("field_of_view", &camera_component::get_fov, &camera_component::set_fov)(
            rttr::metadata("pretty_name", "Field Of View"),
            rttr::metadata("min", 5.0f),
            rttr::metadata("max", 150.0f))
        .property("orthographic_size", &camera_component::get_ortho_size, &camera_component::set_ortho_size)(
            rttr::metadata("pretty_name", "Orthographic Size"),
            rttr::metadata("min", 0.1f),
            rttr::metadata("tooltip",
                           "This is half of the vertical size of the viewing volume.\n"
                           "Horizontal viewing size varies depending on viewport's aspect ratio.\n"
                           "Orthographic size is ignored when camera is not orthographic."))
        .property_readonly("pixels_per_unit", &camera_component::get_ppu)(
            rttr::metadata("pretty_name", "Pixels Per Unit"),
            rttr::metadata("tooltip", "Pixels per unit only usable in orthographic mode."))
        .property_readonly("viewport_size",
                           &camera_component::get_viewport_size)(rttr::metadata("pretty_name", "Viewport Size"))
        .property("near_clip_distance", &camera_component::get_near_clip, &camera_component::set_near_clip)(
            rttr::metadata("pretty_name", "Near Clip"),
            rttr::metadata("min", 0.1f))
        .property("far_clip_distance", &camera_component::get_far_clip, &camera_component::set_far_clip)(
            rttr::metadata("pretty_name", "Far Clip"))
        .property("hdr", &camera_component::get_hdr, &camera_component::set_hdr)(rttr::metadata("pretty_name", "HDR"));
}

SAVE(camera_component)
{
    try_save(ar, ser20::make_nvp("camera", obj.get_camera()));
    try_save(ar, ser20::make_nvp("hdr", obj.get_hdr()));
}
SAVE_INSTANTIATE(camera_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(camera_component, ser20::oarchive_binary_t);

LOAD(camera_component)
{
    try_load(ar, ser20::make_nvp("camera", obj.get_camera()));
    bool hdr{};
    try_load(ar, ser20::make_nvp("hdr", hdr));
    obj.set_hdr(hdr);
}
LOAD_INSTANTIATE(camera_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(camera_component, ser20::iarchive_binary_t);
} // namespace ace
