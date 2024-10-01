#pragma once
#include <engine/rendering/ecs/components/camera_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(camera_component);
LOAD_EXTERN(camera_component);
} // namespace ace
