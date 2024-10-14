#pragma once

#include <engine/scripting/script.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(script);
LOAD_EXTERN(script);
REFLECT_EXTERN(script);

void save_to_file(const std::string& absolute_path, const script::sptr& obj);
void save_to_file_bin(const std::string& absolute_path, const script::sptr& obj);
void load_from_file(const std::string& absolute_path, script::sptr& obj);
void load_from_file_bin(const std::string& absolute_path, script::sptr& obj);

} // namespace ace
