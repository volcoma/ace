#pragma once
#include <engine/ecs/components/camera_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(camera_component);
SAVE_EXTERN(camera_component);
LOAD_EXTERN(camera_component);
}
