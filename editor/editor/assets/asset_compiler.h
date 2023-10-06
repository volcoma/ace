#pragma once
#include <filesystem/filesystem.h>

namespace ace
{
namespace asset_compiler
{

template <typename T>
void compile(const fs::path& key, const fs::path& output_key);
}
}
