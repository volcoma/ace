#include "light.hpp"
#include <engine/meta/core/math/vector.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(light)
{
    rttr::registration::class_<light::spot::shadowmap_params>("light::spot::shadowmap_params");

    rttr::registration::class_<light::spot>("light::spot")(rttr::metadata("pretty_name", "Spot"))
        .property("range", &light::spot::get_range, &light::spot::set_range)(
            rttr::metadata("pretty_name", "Range"),
            rttr::metadata("min", 0.1f),
            rttr::metadata("tooltip", "Light's range from its origin."))
        .property("inner_angle", &light::spot::get_inner_angle, &light::spot::set_inner_angle)(
            rttr::metadata("pretty_name", "Inner Angle"),
            rttr::metadata("min", 1.0f),
            rttr::metadata("max", 85.0f),
            rttr::metadata("step", 0.1f),
            rttr::metadata("tooltip", "Spot light inner cone angle."))
        .property("outer_angle", &light::spot::get_outer_angle, &light::spot::set_outer_angle)(
            rttr::metadata("pretty_name", "Outer Angle"),
            rttr::metadata("min", 1.0f),
            rttr::metadata("max", 90.0f),
            rttr::metadata("step", 0.1f),
            rttr::metadata("tooltip", "Spot light outer cone angle."));

    rttr::registration::class_<light::point::shadowmap_params>("light::point::shadowmap_params")
        .property("fovx_adjust", &light::point::shadowmap_params::fov_x_adjust)(
            rttr::metadata("pretty_name", "FovX Adjust"),
            rttr::metadata("min", -20.0f),
            rttr::metadata("max", 20.0f),
            rttr::metadata("step", 0.0001f),
            rttr::metadata("tooltip", "Shadowmap field of view adjust."))
        .property("fovy_adjust", &light::point::shadowmap_params::fov_y_adjust)(
            rttr::metadata("pretty_name", "FovY Adjust"),
            rttr::metadata("min", -20.0f),
            rttr::metadata("max", 20.0f),
            rttr::metadata("step", 0.0001f),
            rttr::metadata("tooltip", "Shadowmap field of view adjust."))
        .property("stencil_pack", &light::point::shadowmap_params::stencil_pack)(
            rttr::metadata("pretty_name", "Stencil Pack"),
            rttr::metadata("tooltip", "Shadowmap stencil packing algorithm."));

    rttr::registration::class_<light::point>("point")(rttr::metadata("pretty_name", "Point"))
        .property("range", &light::point::range)(rttr::metadata("pretty_name", "Range"),
                                                 rttr::metadata("min", 0.1f),
                                                 rttr::metadata("tooltip", "Light's range from its origin."))
        .property("exponent_falloff", &light::point::exponent_falloff)(
            rttr::metadata("pretty_name", "Exponent Falloff"),
            rttr::metadata("min", 0.1f),
            rttr::metadata("max", 10.0f),
            rttr::metadata("tooltip", "The falloff factor nearing the range edge."));

    rttr::registration::class_<light::directional::shadowmap_params>("light::directional::shadowmap_params")
        .property("splits",
                  &light::directional::shadowmap_params::num_splits)(rttr::metadata("pretty_name", "Splits"),
                                                                     rttr::metadata("min", 1),
                                                                     rttr::metadata("max", 4),
                                                                     rttr::metadata("tooltip", "Number of cascades."))
        .property("distribution", &light::directional::shadowmap_params::split_distribution)(
            rttr::metadata("pretty_name", "Distribution"),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f),
            rttr::metadata("step", 0.001f),
            rttr::metadata("tooltip", "?"))
        .property("stabilize", &light::directional::shadowmap_params::stabilize)(
            rttr::metadata("pretty_name", "Stabilize"),
            rttr::metadata("tooltip", "Stabilize the shadowmaps."));

    rttr::registration::class_<light::directional>("light::directional")(rttr::metadata("pretty_name", "Directional"));

    rttr::registration::enumeration<light_type>("light_type")(rttr::value("Spot", light_type::spot),
                                                              rttr::value("Point", light_type::point),
                                                              rttr::value("Directional", light_type::directional));
    rttr::registration::enumeration<sm_depth>("sm_depth")(rttr::value("InvZ", sm_depth::invz),
                                                          rttr::value("Linear", sm_depth::linear));
    rttr::registration::enumeration<sm_impl>("sm_impl")(rttr::value("Hard", sm_impl::hard),
                                                        rttr::value("Pcf", sm_impl::pcf),
                                                        rttr::value("Pcss", sm_impl::pcss),
                                                        rttr::value("Vsm", sm_impl::vsm),
                                                        rttr::value("Esm", sm_impl::esm));
    rttr::registration::enumeration<sm_resolution>("sm_resolution")(rttr::value("Low", sm_resolution::low),
                                                                    rttr::value("Medium", sm_resolution::medium),
                                                                    rttr::value("High", sm_resolution::high),
                                                                    rttr::value("Very High", sm_resolution::very_high));

    // rttr::registration::class_<light::shadowmap_params::impl_params>("light::shadowmap_params::impl_params")
    //     .property("hardness",
    //               &light::shadowmap_params::impl_params::hardness)(rttr::metadata("pretty_name", "Hardness"),
    //                                                                rttr::metadata("tooltip", "Missing"))
    //     .property("depth_multiplier",
    //               &light::shadowmap_params::impl_params::depth_multiplier)(rttr::metadata("pretty_name", "Depth Multiplier"),
    //                                                                rttr::metadata("tooltip", "Missing"))
    //     .property("blur_x_num",
    //               &light::shadowmap_params::impl_params::blur_x_num)(rttr::metadata("pretty_name", "Blur X Num"),
    //                                                                  rttr::metadata("tooltip", "Missing"))
    //     .property("blur_y_num",
    //               &light::shadowmap_params::impl_params::blur_y_num)(rttr::metadata("pretty_name", "Blur Y Num"),
    //                                                                  rttr::metadata("tooltip", "Missing"))
    //     .property("blur_x_offset",
    //               &light::shadowmap_params::impl_params::blur_x_offset)(rttr::metadata("pretty_name", "Blur X Offset"),
    //                                                                     rttr::metadata("tooltip", "Missing"))
    //     .property("blur_y_offset",
    //               &light::shadowmap_params::impl_params::blur_y_offset)(rttr::metadata("pretty_name", "Blur Y Offset"),
    //                                                                     rttr::metadata("tooltip", "Missing"));

    rttr::registration::class_<light::shadowmap_params>("light::shadowmap_params")

        .property("type", &light::shadowmap_params::type)(rttr::metadata("pretty_name", "Type"),
                                                          rttr::metadata("tooltip", "Shadowmap implementation type."))
        .property("depth",
                  &light::shadowmap_params::depth)(rttr::metadata("pretty_name", "Depth"),
                                                   rttr::metadata("tooltip", "Shadowmap depth pack algorithm."))

        .property("resolution",
                  &light::shadowmap_params::resolution)(rttr::metadata("pretty_name", "Resolution"),
                                                        rttr::metadata("tooltip", "Shadowmap resolution."))
        .property("bias", &light::shadowmap_params::bias)(rttr::metadata("pretty_name", "Bias"),
                                                          rttr::metadata("min", 0.0f),
                                                          rttr::metadata("max", 0.01f),
                                                          rttr::metadata("step", 0.00001f),
                                                          rttr::metadata("tooltip", "Shadowmap bias offset."))
        .property("normal_bias",
                  &light::shadowmap_params::normal_bias)(rttr::metadata("pretty_name", "Normal Bias"),
                                                         rttr::metadata("min", 0.0f),
                                                         rttr::metadata("max", 0.05f),
                                                         rttr::metadata("step", 0.00001f),
                                                         rttr::metadata("tooltip", "Shadowmap normal bias offset"))
        .property("near_plane", &light::shadowmap_params::near_plane)(rttr::metadata("pretty_name", "Near Plane"),
                                                                      rttr::metadata("min", 0.01f),
                                                                      rttr::metadata("max", 10.0f),
                                                                      rttr::metadata("tooltip", "Shadowmap near plane"))
        .property("show_coverage", &light::shadowmap_params::show_coverage)(
            rttr::metadata("pretty_name", "Show Coverage"),
            rttr::metadata("tooltip", "Show shadowmap coverage in view."));

    rttr::registration::class_<light>("light")
        .property("color", &light::color)(rttr::metadata("pretty_name", "Color"),
                                          rttr::metadata("tooltip", "Light's color."))
        .property("intensity", &light::intensity)(rttr::metadata("pretty_name", "Intensity"),
                                                  rttr::metadata("min", 0.0f),
                                                  rttr::metadata("max", 20.0f),
                                                  rttr::metadata("tooltip", "Light's intensity."))
        .property("type", &light::type)(rttr::metadata("pretty_name", "Type"),
                                        rttr::metadata("tooltip", "Light's type."))
        .property("casts_shadows", &light::casts_shadows)(rttr::metadata("pretty_name", "Casts Shadows"),
                                                          rttr::metadata("tooltip", "Is this light casting shadows."));
}

SAVE(light::spot::shadowmap_params)
{
}
SAVE_INSTANTIATE(light::spot::shadowmap_params, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light::spot::shadowmap_params, cereal::oarchive_binary_t);

SAVE(light::spot)
{
    try_save(ar, cereal::make_nvp("range", obj.range));
    try_save(ar, cereal::make_nvp("inner_angle", obj.inner_angle));
    try_save(ar, cereal::make_nvp("outer_angle", obj.outer_angle));
    try_save(ar, cereal::make_nvp("shadow_params", obj.shadow_params));
}
SAVE_INSTANTIATE(light::spot, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light::spot, cereal::oarchive_binary_t);

SAVE(light::point::shadowmap_params)
{
    try_save(ar, cereal::make_nvp("fov_x_adjust", obj.fov_x_adjust));
    try_save(ar, cereal::make_nvp("fov_y_adjust", obj.fov_y_adjust));
    try_save(ar, cereal::make_nvp("stencil_pack", obj.stencil_pack));
}
SAVE_INSTANTIATE(light::point::shadowmap_params, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light::point::shadowmap_params, cereal::oarchive_binary_t);

SAVE(light::point)
{
    try_save(ar, cereal::make_nvp("range", obj.range));
    try_save(ar, cereal::make_nvp("exponent_falloff", obj.exponent_falloff));
    try_save(ar, cereal::make_nvp("shadow_params", obj.shadow_params));
}
SAVE_INSTANTIATE(light::point, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light::point, cereal::oarchive_binary_t);

SAVE(light::directional::shadowmap_params)
{
    try_save(ar, cereal::make_nvp("num_splits", obj.num_splits));
    try_save(ar, cereal::make_nvp("split_distribution", obj.split_distribution));
    try_save(ar, cereal::make_nvp("stabilize", obj.stabilize));
}
SAVE_INSTANTIATE(light::directional::shadowmap_params, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light::directional::shadowmap_params, cereal::oarchive_binary_t);

SAVE(light::directional)
{
    try_save(ar, cereal::make_nvp("shadow_params", obj.shadow_params));
}
SAVE_INSTANTIATE(light::directional, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light::directional, cereal::oarchive_binary_t);

SAVE(light::shadowmap_params)
{
    try_save(ar, cereal::make_nvp("type", obj.type));
    try_save(ar, cereal::make_nvp("depth", obj.depth));
    try_save(ar, cereal::make_nvp("resolution", obj.resolution));
}
SAVE_INSTANTIATE(light::shadowmap_params, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light::shadowmap_params, cereal::oarchive_binary_t);

SAVE(light)
{
    try_save(ar, cereal::make_nvp("type", obj.type));
    try_save(ar, cereal::make_nvp("intensity", obj.intensity));
    try_save(ar, cereal::make_nvp("color", obj.color));
    try_save(ar, cereal::make_nvp("casts_shadows", obj.casts_shadows));

    try_save(ar, cereal::make_nvp("shadow_params", obj.shadow_params));

    if(obj.type == light_type::spot)
    {
        try_save(ar, cereal::make_nvp("spot_data", obj.spot_data));
    }
    else if(obj.type == light_type::point)
    {
        try_save(ar, cereal::make_nvp("point_data", obj.point_data));
    }
    else if(obj.type == light_type::directional)
    {
        try_save(ar, cereal::make_nvp("directional_data", obj.directional_data));
    }
}
SAVE_INSTANTIATE(light, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light, cereal::oarchive_binary_t);

LOAD(light::spot::shadowmap_params)
{
}
LOAD_INSTANTIATE(light::spot::shadowmap_params, cereal::oarchive_associative_t);
LOAD_INSTANTIATE(light::spot::shadowmap_params, cereal::oarchive_binary_t);

LOAD(light::spot)
{
    try_load(ar, cereal::make_nvp("range", obj.range));
    try_load(ar, cereal::make_nvp("inner_angle", obj.inner_angle));
    try_load(ar, cereal::make_nvp("outer_angle", obj.outer_angle));
    try_load(ar, cereal::make_nvp("shadow_params", obj.shadow_params));
}
LOAD_INSTANTIATE(light::spot, cereal::oarchive_associative_t);
LOAD_INSTANTIATE(light::spot, cereal::oarchive_binary_t);

LOAD(light::point::shadowmap_params)
{
    try_load(ar, cereal::make_nvp("fov_x_adjust", obj.fov_x_adjust));
    try_load(ar, cereal::make_nvp("fov_y_adjust", obj.fov_y_adjust));
    try_load(ar, cereal::make_nvp("stencil_pack", obj.stencil_pack));
}
LOAD_INSTANTIATE(light::point::shadowmap_params, cereal::oarchive_associative_t);
LOAD_INSTANTIATE(light::point::shadowmap_params, cereal::oarchive_binary_t);

LOAD(light::point)
{
    try_load(ar, cereal::make_nvp("range", obj.range));
    try_load(ar, cereal::make_nvp("exponent_falloff", obj.exponent_falloff));
    try_load(ar, cereal::make_nvp("shadow_params", obj.shadow_params));
}
LOAD_INSTANTIATE(light::point, cereal::oarchive_associative_t);
LOAD_INSTANTIATE(light::point, cereal::oarchive_binary_t);

LOAD(light::directional::shadowmap_params)
{
    try_load(ar, cereal::make_nvp("num_splits", obj.num_splits));
    try_load(ar, cereal::make_nvp("split_distribution", obj.split_distribution));
    try_load(ar, cereal::make_nvp("stabilize", obj.stabilize));
}
LOAD_INSTANTIATE(light::directional::shadowmap_params, cereal::oarchive_associative_t);
LOAD_INSTANTIATE(light::directional::shadowmap_params, cereal::oarchive_binary_t);

LOAD(light::directional)
{
    try_load(ar, cereal::make_nvp("shadow_params", obj.shadow_params));
}
LOAD_INSTANTIATE(light::directional, cereal::oarchive_associative_t);
LOAD_INSTANTIATE(light::directional, cereal::oarchive_binary_t);

LOAD(light::shadowmap_params)
{
    try_load(ar, cereal::make_nvp("type", obj.type));
    try_load(ar, cereal::make_nvp("depth", obj.depth));
    try_load(ar, cereal::make_nvp("resolution", obj.resolution));
}
LOAD_INSTANTIATE(light::shadowmap_params, cereal::oarchive_associative_t);
LOAD_INSTANTIATE(light::shadowmap_params, cereal::oarchive_binary_t);

LOAD(light)
{
    try_load(ar, cereal::make_nvp("type", obj.type));
    try_load(ar, cereal::make_nvp("intensity", obj.intensity));
    try_load(ar, cereal::make_nvp("color", obj.color));
    try_load(ar, cereal::make_nvp("casts_shadows", obj.casts_shadows));
    try_load(ar, cereal::make_nvp("shadow_params", obj.shadow_params));

    if(obj.type == light_type::spot)
    {
        try_load(ar, cereal::make_nvp("spot_data", obj.spot_data));
    }
    else if(obj.type == light_type::point)
    {
        try_load(ar, cereal::make_nvp("point_data", obj.point_data));
    }
    else if(obj.type == light_type::directional)
    {
        try_load(ar, cereal::make_nvp("directional_data", obj.directional_data));
    }
}
LOAD_INSTANTIATE(light, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(light, cereal::iarchive_binary_t);
} // namespace ace
