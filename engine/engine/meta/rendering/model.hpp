#pragma once

#include <engine/rendering/model.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(model);
SAVE_EXTERN(model);
LOAD_EXTERN(model);
}
