#include "asset_writer.h"

#include <engine/meta/rendering/material.hpp>
#include <engine/meta/rendering/standard_material.hpp>
#include <engine/meta/physics/physics_material.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
namespace asset_writer
{

template<>
void save_to_file<material>(const fs::path& key, const asset_handle<material>& asset)
{
    fs::path absolute_key = fs::absolute(fs::resolve_protocol(key));
    save_to_file(absolute_key.string(), asset.get_ptr());
}

template<>
void save_to_file<physics_material>(const fs::path& key, const asset_handle<physics_material>& asset)
{
    fs::path absolute_key = fs::absolute(fs::resolve_protocol(key));
    save_to_file(absolute_key.string(), asset.get());
}
} // namespace asset_writer
} // namespace ace
