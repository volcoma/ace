#pragma once

#include <engine/ecs/components/transform_component.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{

struct relationship_component
{
    entt::handle parent;
    std::vector<entt::handle> children;
};

REFLECT_EXTERN(transform_component);
SAVE_EXTERN(transform_component);
LOAD_EXTERN(transform_component);
}
