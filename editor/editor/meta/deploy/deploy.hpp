#pragma once
#include <editor/deploy/deploy.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(deploy_settings);
LOAD_EXTERN(deploy_settings);
REFLECT_EXTERN(deploy_settings);

void save_to_file(const std::string& absolute_path, const deploy_settings& obj);
void save_to_file_bin(const std::string& absolute_path, const deploy_settings& obj);
auto load_from_file(const std::string& absolute_path, deploy_settings& obj) -> bool;
auto load_from_file_bin(const std::string& absolute_path, deploy_settings& obj) -> bool;

}
