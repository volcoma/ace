#pragma once
#include <engine/ecs/components/model_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(model_component);
LOAD_EXTERN(model_component);

SAVE_EXTERN(bone_component);
LOAD_EXTERN(bone_component);

SAVE_EXTERN(submesh_component);
LOAD_EXTERN(submesh_component);
} // namespace ace
