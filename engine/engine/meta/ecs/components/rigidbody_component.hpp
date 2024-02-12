#pragma once

#include <engine/ecs/components/rigidbody_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{

REFLECT_EXTERN(rigidbody_component);
SAVE_EXTERN(rigidbody_component);
LOAD_EXTERN(rigidbody_component);
} // namespace ace
