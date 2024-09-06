#pragma once
#include <engine/engine_export.h>

#include <engine/assets/asset_manager.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{

SAVE_EXTERN(asset_meta);
LOAD_EXTERN(asset_meta);

SAVE_EXTERN(asset_database::meta);
LOAD_EXTERN(asset_database::meta);

SAVE_EXTERN(asset_database);
LOAD_EXTERN(asset_database);

void save_to_file(const std::string& absolute_path, const asset_database& obj);
void save_to_file_bin(const std::string& absolute_path, const asset_database& obj);
auto load_from_file(const std::string& absolute_path, asset_database& obj) -> bool;
auto load_from_file_bin(const std::string& absolute_path, asset_database& obj) -> bool;


void save_to_file(const std::string& absolute_path, const asset_meta& obj);
void save_to_file_bin(const std::string& absolute_path, const asset_meta& obj);
auto load_from_file(const std::string& absolute_path, asset_meta& obj) -> bool;
auto load_from_file_bin(const std::string& absolute_path, asset_meta& obj) -> bool;
} // namespace ace
