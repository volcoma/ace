#pragma once

#include <engine/audio/audio_clip.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(audio_clip);
SAVE_EXTERN(audio_clip);
LOAD_EXTERN(audio_clip);

void save_to_file(const std::string& absolute_path, const audio::sound_data& obj);
void save_to_file_bin(const std::string& absolute_path, const audio::sound_data& obj);
auto load_from_file(const std::string& absolute_path, audio::sound_data& obj, std::string& err) -> bool;
void load_from_file_bin(const std::string& absolute_path, audio::sound_data& obj);

} // namespace ace
