#include "standard_material.hpp"
#include "material.hpp"

#include <engine/meta/assets/asset_handle.hpp>
#include <engine/meta/core/math/vector.hpp>

#include <serialization/types/string.hpp>
#include <serialization/types/unordered_map.hpp>

#include <serialization/types/utility.hpp>

namespace ace
{

REFLECT(pbr_material)
{
    rttr::registration::class_<pbr_material>("pbr_material")
        .property("base_color", &pbr_material::get_base_color, &pbr_material::set_base_color)(
            rttr::metadata("pretty_name", "Base Color"))
        .property("", &pbr_material::get_subsurface_color, &pbr_material::set_subsurface_color)(
            rttr::metadata("pretty_name", "Subsurface Color"))
        .property("emissive_color", &pbr_material::get_emissive_color, &pbr_material::set_emissive_color)(
            rttr::metadata("pretty_name", "Emissive Color"))
        .property("roughness", &pbr_material::get_roughness, &pbr_material::set_roughness)(
            rttr::metadata("pretty_name", "Roughness"),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("metalness", &pbr_material::get_metalness, &pbr_material::set_metalness)(
            rttr::metadata("pretty_name", "Metalness"),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("bumpiness", &pbr_material::get_bumpiness, &pbr_material::set_bumpiness)(
            rttr::metadata("pretty_name", "Bumpiness"),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 10.0f))
        .property("alpha_test_value", &pbr_material::get_alpha_test_value, &pbr_material::set_alpha_test_value)(
            rttr::metadata("pretty_name", "Alpha Test Value"),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("tiling", &pbr_material::get_tiling, &pbr_material::set_tiling)(
            rttr::metadata("pretty_name", "Tiling"))
        .property("dither_threshold", &pbr_material::get_dither_threshold, &pbr_material::set_dither_threshold)(
            rttr::metadata("pretty_name", "Dither Threshold"))
        .property("color_map", &pbr_material::get_color_map, &pbr_material::set_color_map)(
            rttr::metadata("pretty_name", "Color Map"))
        .property("normal_map", &pbr_material::get_normal_map, &pbr_material::set_normal_map)(
            rttr::metadata("pretty_name", "Normal Map"))
        .property("roughness_map", &pbr_material::get_roughness_map, &pbr_material::set_roughness_map)(
            rttr::metadata("pretty_name", "Roughness Map"),
            rttr::metadata("tooltip", "Red Channel (R): Contains the roughness values.\n"
                                      "When Metalness and Roughness maps are the same.\n"
                                      "As per glTF 2.0 specification:\n"
                                      "Green Channel (G): Contains the roughness values.\n"
                                      "Blue Channel (B): Contains the metalness values."))
        .property("metalness_map", &pbr_material::get_metalness_map, &pbr_material::set_metalness_map)(
            rttr::metadata("pretty_name", "Metalness Map"),
            rttr::metadata("tooltip",
                           "Red Channel (R): Contains the metalness values.\n"
                           "When Metalness and Roughness maps are the same.\n"
                           "As per glTF 2.0 specification:\n"
                           "Green Channel (G): Contains the roughness values.\n"
                           "Blue Channel (B): Contains the metalness values."))
        .property("emissive_map", &pbr_material::get_emissive_map, &pbr_material::set_emissive_map)(
            rttr::metadata("pretty_name", "Emissive Map"),
            rttr::metadata("tooltip", "emissive color map."))
        .property("ao_map", &pbr_material::get_ao_map, &pbr_material::set_ao_map)(
            rttr::metadata("pretty_name", "AO Map"),
            rttr::metadata("tooltip", "black/white texture."));
}

SAVE(pbr_material)
{
    try_save(ar, ser20::make_nvp("base_type", ser20::base_class<material>(&obj)));
    try_save(ar, ser20::make_nvp("base_color", obj.base_color_));
    try_save(ar, ser20::make_nvp("subsurface_color", obj.subsurface_color_));
    try_save(ar, ser20::make_nvp("emissive_color", obj.emissive_color_));
    try_save(ar, ser20::make_nvp("surface_data", obj.surface_data_));
    try_save(ar, ser20::make_nvp("tiling", obj.tiling_));
    try_save(ar, ser20::make_nvp("dither_threshold", obj.dither_threshold_));

    try_save(ar, ser20::make_nvp("color_map", obj.color_map_));
    try_save(ar, ser20::make_nvp("normal_map", obj.normal_map_));
    try_save(ar, ser20::make_nvp("roughness_map", obj.roughness_map_));
    try_save(ar, ser20::make_nvp("metalness_map", obj.metalness_map_));
    try_save(ar, ser20::make_nvp("emissive_map", obj.emissive_map_));
    try_save(ar, ser20::make_nvp("ao_map", obj.ao_map_));
}
SAVE_INSTANTIATE(pbr_material, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(pbr_material, ser20::oarchive_binary_t);

LOAD(pbr_material)
{
    try_load(ar, ser20::make_nvp("base_type", ser20::base_class<material>(&obj)));
    try_load(ar, ser20::make_nvp("base_color", obj.base_color_));
    try_load(ar, ser20::make_nvp("subsurface_color", obj.subsurface_color_));
    try_load(ar, ser20::make_nvp("emissive_color", obj.emissive_color_));
    try_load(ar, ser20::make_nvp("surface_data", obj.surface_data_));
    try_load(ar, ser20::make_nvp("tiling", obj.tiling_));
    try_load(ar, ser20::make_nvp("dither_threshold", obj.dither_threshold_));

    try_load(ar, ser20::make_nvp("color_map", obj.color_map_));
    try_load(ar, ser20::make_nvp("normal_map", obj.normal_map_));
    try_load(ar, ser20::make_nvp("roughness_map", obj.roughness_map_));
    try_load(ar, ser20::make_nvp("metalness_map", obj.metalness_map_));
    try_load(ar, ser20::make_nvp("emissive_map", obj.emissive_map_));
    try_load(ar, ser20::make_nvp("ao_map", obj.ao_map_));
}
LOAD_INSTANTIATE(pbr_material, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(pbr_material, ser20::iarchive_binary_t);
} // namespace ace
