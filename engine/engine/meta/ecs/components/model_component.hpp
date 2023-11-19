#pragma once
#include <engine/ecs/components/model_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(model_component);
SAVE_EXTERN(model_component);
LOAD_EXTERN(model_component);
}
