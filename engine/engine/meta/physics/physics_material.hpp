#pragma once

#include <engine/physics/physics_material.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(physics_material);
SAVE_EXTERN(physics_material);
LOAD_EXTERN(physics_material);

void save_to_file(const std::string& absolute_path, const physics_material::sptr& obj);
void save_to_file_bin(const std::string& absolute_path, const physics_material::sptr& obj);
void load_from_file(const std::string& absolute_path, physics_material::sptr& obj);
void load_from_file_bin(const std::string& absolute_path, physics_material::sptr& obj);

} // namespace ace
