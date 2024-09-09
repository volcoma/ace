#include "settings.hpp"

#include <fstream>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include <engine/meta/assets/asset_handle.hpp>
#include <engine/meta/ecs/entity.hpp>

namespace ace
{

using app_settings_t = settings::app_settings;
REFLECT(app_settings_t)
{
    rttr::registration::class_<settings::app_settings>("app_settings")(rttr::metadata("pretty_name", "Application"))
        .constructor<>()()
        .property("company", &settings::app_settings::company)(rttr::metadata("pretty_name", "Company"),
                                                               rttr::metadata("tooltip", "Missing..."))
        .property("product", &settings::app_settings::product)(rttr::metadata("pretty_name", "Product"),
                                                               rttr::metadata("tooltip", "Missing..."))
        .property("version", &settings::app_settings::version)(rttr::metadata("pretty_name", "Version"),
                                                               rttr::metadata("tooltip", "Missing..."));
}

SAVE_INLINE(settings::app_settings)
{
    try_save(ar, ser20::make_nvp("company", obj.company));
    try_save(ar, ser20::make_nvp("product", obj.product));
    try_save(ar, ser20::make_nvp("version", obj.version));
}

LOAD_INLINE(settings::app_settings)
{
    try_load(ar, ser20::make_nvp("company", obj.company));
    try_load(ar, ser20::make_nvp("product", obj.product));
    try_load(ar, ser20::make_nvp("version", obj.version));
}

using graphics_settings_t = settings::graphics_settings;
REFLECT(graphics_settings_t)
{
    rttr::registration::class_<settings::graphics_settings>("graphics_settings")(
        rttr::metadata("pretty_name", "Graphics"))
        .constructor<>()();
}

SAVE_INLINE(settings::graphics_settings)
{
    // try_save(ar, ser20::make_nvp("company", obj.company));
    // try_save(ar, ser20::make_nvp("product", obj.product));
    // try_save(ar, ser20::make_nvp("version", obj.version));
}

LOAD_INLINE(settings::graphics_settings)
{
    // try_load(ar, ser20::make_nvp("company", obj.company));
    // try_load(ar, ser20::make_nvp("product", obj.product));
    // try_load(ar, ser20::make_nvp("version", obj.version));
}

using standalone_settings_t = settings::standalone_settings;
REFLECT(standalone_settings_t)
{
    rttr::registration::class_<settings::standalone_settings>("standalone_settings")(
        rttr::metadata("pretty_name", "Standalone"))
        .constructor<>()()
        .property("startup_scene",
                  &settings::standalone_settings::startup_scene)(rttr::metadata("pretty_name", "Startup Scene"),
                                                                 rttr::metadata("tooltip", "The scene to load first."));
}

SAVE_INLINE(settings::standalone_settings)
{
    try_save(ar, ser20::make_nvp("startup_scene", obj.startup_scene));
}

LOAD_INLINE(settings::standalone_settings)
{
    try_load(ar, ser20::make_nvp("startup_scene", obj.startup_scene));
}

REFLECT(settings)
{
    rttr::registration::class_<settings>("settings")(rttr::metadata("pretty_name", "Settings"))
        .constructor<>()()
        .property("app", &settings::app)(rttr::metadata("pretty_name", "Application"),
                                         rttr::metadata("tooltip", "Missing..."))
        .property("graphics", &settings::graphics)(rttr::metadata("pretty_name", "Graphics"),
                                                   rttr::metadata("tooltip", "Missing..."))
        .property("standalone", &settings::standalone)(rttr::metadata("pretty_name", "Standalone"),
                                                       rttr::metadata("tooltip", "Missing..."));
}

SAVE(settings)
{
    try_save(ar, ser20::make_nvp("app", obj.app));
    try_save(ar, ser20::make_nvp("graphics", obj.graphics));
    try_save(ar, ser20::make_nvp("standalone", obj.standalone));
}
SAVE_INSTANTIATE(settings, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(settings, ser20::oarchive_binary_t);

LOAD(settings)
{
    try_load(ar, ser20::make_nvp("app", obj.app));
    try_load(ar, ser20::make_nvp("graphics", obj.graphics));
    try_load(ar, ser20::make_nvp("standalone", obj.standalone));
}
LOAD_INSTANTIATE(settings, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(settings, ser20::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const settings& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        ser20::oarchive_associative_t ar(stream);
        try_save(ar, ser20::make_nvp("settings", obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const settings& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("settings", obj));
    }
}

auto load_from_file(const std::string& absolute_path, settings& obj) -> bool
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        ser20::iarchive_associative_t ar(stream);
        return try_load(ar, ser20::make_nvp("settings", obj));
    }

    return false;
}

auto load_from_file_bin(const std::string& absolute_path, settings& obj) -> bool
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::iarchive_binary_t ar(stream);
        return try_load(ar, ser20::make_nvp("settings", obj));
    }

    return false;
}
} // namespace ace
