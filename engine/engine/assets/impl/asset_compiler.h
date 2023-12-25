#pragma once
#include <filesystem/filesystem.h>
#include <engine/assets/asset_manager.h>

namespace ace
{
namespace asset_compiler
{

template <typename T>
void compile(asset_manager& am, const fs::path& key, const fs::path& output_key);
}
}
