#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/assets/asset_handle.h>
#include <entt/entt.hpp>

#include "prefab.h"
using namespace entt::literals;

namespace ace
{

struct scene
{
    scene();
    ~scene();

    auto instantiate(const asset_handle<prefab>& pfb) -> entt::handle;
    auto create_entity(entt::entity e) -> entt::handle;
    auto create_entity(entt::entity e) const -> entt::const_handle;

    auto create_entity(const std::string& tag = {}, entt::handle parent = {}) -> entt::handle;
    auto clone_entity(entt::handle e, bool keep_parent = true) -> entt::handle;

    static void clone_scene(const scene& src_scene, scene& dst_scene);

    void unload();

    std::unique_ptr<entt::registry> registry{};
};

} // namespace ace
