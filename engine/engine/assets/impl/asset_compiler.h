#pragma once
#include <engine/assets/asset_manager.h>
#include <filesystem/filesystem.h>

namespace ace
{
namespace asset_compiler
{

template<typename T>
void compile(asset_manager& am, const fs::path& key, const fs::path& output_key);

} // namespace asset_compiler
} // namespace ace
