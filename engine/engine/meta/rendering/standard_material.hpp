#pragma once
#include <engine/rendering/material.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(standard_material);

SAVE_EXTERN(standard_material);
LOAD_EXTERN(standard_material);
}

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>
CEREAL_REGISTER_TYPE_WITH_NAME(ace::standard_material, "standard_material")
