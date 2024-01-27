#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include "impl/entt.hpp"
#include "prefab.h"
#include <engine/assets/asset_handle.h>

using namespace entt::literals;

namespace ace
{

struct scene
{
    scene();

    auto instantiate(const asset_handle<prefab>& pfb) -> entt::handle;
    auto create_entity(entt::entity e) -> entt::handle;
    auto create_entity(entt::entity e) const -> entt::const_handle;

    auto create_entity(const std::string& tag = {}, entt::handle parent = {}) -> entt::handle;
    auto clone_entity(entt::handle e, bool keep_parent = true) -> entt::handle;

    void unload();

    entt::registry registry{};
};

struct ecs
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void on_frame_render(rtti::context& ctx, delta_t dt);

    void unload_scene();
    auto get_scene() -> scene&;

private:
    scene scene_{};
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};

} // namespace ace
