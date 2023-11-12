#pragma once

#include <engine/rendering/camera.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(camera);
SAVE_EXTERN(camera);
LOAD_EXTERN(camera);
}
