#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include "impl/entt.hpp"
#include "prefab.h"
#include <engine/assets/asset_handle.h>

using namespace entt::literals;

namespace ace
{

struct ecs
{
    ecs();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);

    void close_project();

    auto instantiate(const asset_handle<prefab>& pfb) -> entt::handle;

    auto create_entity(entt::handle parent = {}) -> entt::handle;
    auto clone_entity(entt::handle e, bool keep_parent = true) -> entt::handle;

    auto create_test_scene() -> entt::handle;

    entt::registry registry{};

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

};

} // namespace ace
