#pragma once

#include <audiopp/sound.h>
#include <audiopp/sound_data.h>

namespace ace
{

/**
 * @brief Struct representing an audio clip.
 *
 * This struct inherits from `audio::sound` and provides additional functionality for audio clips.
 */
struct audio_clip : public audio::sound
{
    using base_type = audio::sound;
    using base_type::base_type;
};

} // namespace ace
