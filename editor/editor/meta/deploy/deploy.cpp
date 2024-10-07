#include "deploy.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include <serialization/types/array.hpp>

namespace ace
{
REFLECT(deploy_settings)
{
    rttr::registration::class_<deploy_settings>("deploy_settings")(rttr::metadata("pretty_name", "Deploy Options"))
        .constructor<>()
        .property("deploy_location",
                  &deploy_settings::deploy_location)(rttr::metadata("pretty_name", "Deploy Location"),
                                                   rttr::metadata("tooltip", "Choose the deploy location."))
        .property("deploy_dependencies", &deploy_settings::deploy_dependencies)(
            rttr::metadata("pretty_name", "Deploy Dependencies"),
            rttr::metadata("tooltip", "This takes some time and if already done should't be necessary."))
        .property("run",
                  &deploy_settings::deploy_and_run)(rttr::metadata("pretty_name", "Deploy & Run"),
                                                  rttr::metadata("tooltip", "Run the application after the deploy."));
}

SAVE(deploy_settings)
{
    try_save(ar, ser20::make_nvp("deploy_location", obj.deploy_location.generic_string()));
    try_save(ar, ser20::make_nvp("deploy_dependencies", obj.deploy_dependencies));
    try_save(ar, ser20::make_nvp("deploy_and_run", obj.deploy_and_run));
}
SAVE_INSTANTIATE(deploy_settings, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(deploy_settings, ser20::oarchive_binary_t);

LOAD(deploy_settings)
{
    std::string deploy_location;
    if(try_load(ar, ser20::make_nvp("deploy_location", deploy_location)))
    {
        obj.deploy_location = deploy_location;
    }
    try_load(ar, ser20::make_nvp("deploy_dependencies", obj.deploy_dependencies));
    try_load(ar, ser20::make_nvp("deploy_and_run", obj.deploy_and_run));
}
LOAD_INSTANTIATE(deploy_settings, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(deploy_settings, ser20::iarchive_binary_t);


void save_to_file(const std::string& absolute_path, const deploy_settings& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        auto ar = ser20::create_oarchive_associative(stream);
        try_save(ar, ser20::make_nvp("settings", obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const deploy_settings& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("settings", obj));
    }
}

auto load_from_file(const std::string& absolute_path, deploy_settings& obj) -> bool
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        auto ar = ser20::create_iarchive_associative(stream);
        return try_load(ar, ser20::make_nvp("settings", obj));
    }

    return false;
}

auto load_from_file_bin(const std::string& absolute_path, deploy_settings& obj) -> bool
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
