#pragma once
#include <engine/ecs/components/animation_component.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(animation_component);
SAVE_EXTERN(animation_component);
LOAD_EXTERN(animation_component);

} // namespace ace
