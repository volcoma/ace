#pragma once

#include <engine/settings/settings.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(settings);
LOAD_EXTERN(settings);

void save_to_file(const std::string& absolute_path, const settings& obj);
void save_to_file_bin(const std::string& absolute_path, const settings& obj);
auto load_from_file(const std::string& absolute_path, settings& obj) -> bool;
auto load_from_file_bin(const std::string& absolute_path, settings& obj) -> bool;

} // namespace ace
