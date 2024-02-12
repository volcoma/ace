#pragma once

#include <engine/ecs/components/box_collider_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{

REFLECT_EXTERN(box_collider_component);
SAVE_EXTERN(box_collider_component);
LOAD_EXTERN(box_collider_component);
} // namespace ace
