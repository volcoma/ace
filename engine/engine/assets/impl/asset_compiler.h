#pragma once
#include <filesystem/filesystem.h>
#include <engine/assets/asset_manager.h>

namespace ace
{
namespace asset_compiler
{

template <typename T>
void compile(asset_manager& am, const fs::path& key, const fs::path& output_key);

auto run_process(const std::string& process, const std::vector<std::string>& args_array, std::string& err) -> bool;
}
}
