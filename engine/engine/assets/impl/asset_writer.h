#pragma once
#include "../asset_handle.h"

#include <filesystem/filesystem.h>

namespace ace
{
namespace asset_writer
{
template <typename T>
extern void save_to_file(const fs::path& key, const asset_handle<T>& asset);
}
}
