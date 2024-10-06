#pragma once
#include <engine/ecs/components/test_component.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(test_component);
LOAD_EXTERN(test_component);
REFLECT_EXTERN(test_component);
} // namespace ace
