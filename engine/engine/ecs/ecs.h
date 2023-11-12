#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include "impl/entt.hpp"

using namespace entt::literals;

namespace ace
{

struct ecs
{
    ecs();

    void close_project();

    auto create_entity(entt::handle parent = {}) -> entt::handle;

    auto create_test_scene() -> entt::handle;

    entt::registry registry{};
};
} // namespace ace
