#pragma once

#include <engine/rendering/light.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(light);
SAVE_EXTERN(light);
LOAD_EXTERN(light);
} // namespace ace
