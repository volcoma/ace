#pragma once

#include <audiopp/sound_data.h>
#include <audiopp/sound.h>

namespace ace
{

struct audio_clip : public audio::sound
{
    using base_type = audio::sound;
    using base_type::base_type;
};

} // namespace ace
