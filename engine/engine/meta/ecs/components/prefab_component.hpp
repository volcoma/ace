#pragma once
#include <engine/ecs/components/prefab_component.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{

SAVE_EXTERN(prefab_component);
LOAD_EXTERN(prefab_component);
} // namespace ace
