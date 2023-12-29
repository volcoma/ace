#pragma once
#include <engine/ecs/ecs.h>
#include <engine/ecs/prefab.h>
#include <engine/assets/asset_handle.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{

void save_to_file(const std::string& absolute_path, entt::handle obj);
void save_to_file_bin(const std::string& absolute_path, entt::handle obj);
void load_from_file(const std::string& absolute_path, entt::handle& obj);
void load_from_file_bin(const std::string& absolute_path, entt::handle& obj);

auto load_from_prefab(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle;
auto load_from_prefab_bin(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle;

} // namespace ace

namespace cereal
{

SAVE_EXTERN(entt::handle);
LOAD_EXTERN(entt::handle);
} // namespace cereal
