#pragma once
#include <engine/audio/ecs/components/audio_listener_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(audio_listener_component);
LOAD_EXTERN(audio_listener_component);
REFLECT_EXTERN(audio_listener_component);

} // namespace ace
