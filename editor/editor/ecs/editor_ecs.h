#pragma once
#include <engine/ecs/ecs.h>

namespace ace
{

struct editor_ecs : ecs
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    auto create_editor_camera() -> entt::handle;

    entt::handle editor_camera;
};
} // namespace ace
