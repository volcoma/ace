#include "asset_writer.h"

#include <engine/meta/rendering/material.hpp>
#include <engine/meta/rendering/standard_material.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include <fstream>
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
} // namespace asset_writer
} // namespace ace
