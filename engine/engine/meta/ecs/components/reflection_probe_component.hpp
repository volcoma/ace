#pragma once
#include <engine/ecs/components/reflection_probe_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(reflection_probe_component);
SAVE_EXTERN(reflection_probe_component);
LOAD_EXTERN(reflection_probe_component);
} // namespace ace
