#pragma once

#include <engine/ecs/components/transform_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(transform_component);
SAVE_EXTERN(transform_component);
LOAD_EXTERN(transform_component);
}

//#include <serialization/associative_archive.h>
//#include <serialization/binary_archive.h>
//CEREAL_REGISTER_TYPE(ace::transform_component)
