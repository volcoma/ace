#pragma once
#include <engine/ecs/components/light_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(light_component);
SAVE_EXTERN(light_component);
LOAD_EXTERN(light_component);

REFLECT_EXTERN(skylight_component);
SAVE_EXTERN(skylight_component);
LOAD_EXTERN(skylight_component);
} // namespace ace
