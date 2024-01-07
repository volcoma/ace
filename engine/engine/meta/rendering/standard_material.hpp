#pragma once
#include <engine/rendering/material.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(pbr_material);

SAVE_EXTERN(pbr_material);
LOAD_EXTERN(pbr_material);
}

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>
CEREAL_REGISTER_TYPE_WITH_NAME(ace::pbr_material, "pbr_material")
