#pragma once
#include <engine/animation/ecs/components/animation_component.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(animation_component);
LOAD_EXTERN(animation_component);
REFLECT_EXTERN(animation_component);

} // namespace ace
