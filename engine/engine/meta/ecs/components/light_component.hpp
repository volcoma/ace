#pragma once
#include <engine/rendering/ecs/components/light_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(light_component);
LOAD_EXTERN(light_component);

SAVE_EXTERN(skylight_component);
LOAD_EXTERN(skylight_component);
} // namespace ace
