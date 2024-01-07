#pragma once
#include <engine/ecs/components/id_component.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(id_component);
SAVE_EXTERN(id_component);
LOAD_EXTERN(id_component);

REFLECT_EXTERN(tag_component);
SAVE_EXTERN(tag_component);
LOAD_EXTERN(tag_component);
} // namespace ace
