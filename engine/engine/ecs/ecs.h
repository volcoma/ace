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

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void close_project();

    auto create_editor_camera() -> entt::handle;
    auto create_entity(entt::handle parent = {}) -> entt::handle;

    auto create_test_scene() -> entt::handle;

    entt::registry registry{};
    entt::handle editor_camera;
};
} // namespace ace
