#pragma once

#include <engine/rendering/camera.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(camera);
LOAD_EXTERN(camera);
REFLECT_EXTERN(camera);

} // namespace ace
