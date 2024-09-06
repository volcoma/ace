#pragma once
#include <engine/rendering/material.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(material);
LOAD_EXTERN(material);

void save_to_file(const std::string& absolute_path, const std::shared_ptr<material>& obj);
void save_to_file_bin(const std::string& absolute_path, const std::shared_ptr<material>& obj);
void load_from_file(const std::string& absolute_path, std::shared_ptr<material>& obj);
void load_from_file_bin(const std::string& absolute_path, std::shared_ptr<material>& obj);

} // namespace ace
