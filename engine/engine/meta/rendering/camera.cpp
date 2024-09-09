#include "camera.hpp"
#include <engine/meta/core/common/basetypes.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(camera)
{
    rttr::registration::enumeration<projection_mode>("projection_mode")(
        rttr::value("Perspective", projection_mode::perspective),
        rttr::value("Orthographic", projection_mode::orthographic));
    rttr::registration::class_<camera>("camera");
}

SAVE(camera)
{
    try_save(ar, ser20::make_nvp("projection_mode", obj.projection_mode_));
    try_save(ar, ser20::make_nvp("field_of_view", obj.fov_));
    try_save(ar, ser20::make_nvp("near_clip", obj.near_clip_));
    try_save(ar, ser20::make_nvp("far_clip", obj.far_clip_));
    try_save(ar, ser20::make_nvp("viewport_position", obj.viewport_pos_));
    try_save(ar, ser20::make_nvp("viewport_size", obj.viewport_size_));
    try_save(ar, ser20::make_nvp("orthographic_size", obj.ortho_size_));
    try_save(ar, ser20::make_nvp("aspect_ratio", obj.aspect_ratio_));
    try_save(ar, ser20::make_nvp("aspect_locked", obj.aspect_locked_));
    try_save(ar, ser20::make_nvp("frustum_locked", obj.frustum_locked_));
}
SAVE_INSTANTIATE(camera, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(camera, ser20::oarchive_binary_t);

LOAD(camera)
{
    try_load(ar, ser20::make_nvp("projection_mode", obj.projection_mode_));
    try_load(ar, ser20::make_nvp("field_of_view", obj.fov_));
    try_load(ar, ser20::make_nvp("near_clip", obj.near_clip_));
    try_load(ar, ser20::make_nvp("far_clip", obj.far_clip_));
    try_load(ar, ser20::make_nvp("viewport_position", obj.viewport_pos_));
    try_load(ar, ser20::make_nvp("viewport_size", obj.viewport_size_));
    try_load(ar, ser20::make_nvp("orthographic_size", obj.ortho_size_));
    try_load(ar, ser20::make_nvp("aspect_ratio", obj.aspect_ratio_));
    try_load(ar, ser20::make_nvp("aspect_locked", obj.aspect_locked_));
    try_load(ar, ser20::make_nvp("frustum_locked", obj.frustum_locked_));

    obj.view_dirty_ = true;
    obj.projection_dirty_ = true;
    obj.aspect_dirty_ = true;
    obj.frustum_dirty_ = true;
}
LOAD_INSTANTIATE(camera, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(camera, ser20::iarchive_binary_t);
} // namespace ace
