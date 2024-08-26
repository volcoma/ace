#pragma once
#include <engine/ecs/components/model_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(model_component);
SAVE_EXTERN(model_component);
LOAD_EXTERN(model_component);

REFLECT_EXTERN(bone_component);
SAVE_EXTERN(bone_component);
LOAD_EXTERN(bone_component);

REFLECT_EXTERN(subset_component);
SAVE_EXTERN(subset_component);
LOAD_EXTERN(subset_component);
} // namespace ace
