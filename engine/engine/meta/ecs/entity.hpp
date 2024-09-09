#pragma once
#include <engine/assets/asset_handle.h>
#include <engine/ecs/scene.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{

void save_to_stream(std::ostream& stream, entt::const_handle obj);
void save_to_file(const std::string& absolute_path, entt::const_handle obj);
void save_to_stream_bin(std::ostream& stream, entt::const_handle obj);
void save_to_file_bin(const std::string& absolute_path, entt::const_handle obj);

void load_from_stream(std::istream& stream, entt::handle& obj);
void load_from_file(const std::string& absolute_path, entt::handle& obj);
void load_from_stream_bin(std::istream& stream, entt::handle& obj);
void load_from_file_bin(const std::string& absolute_path, entt::handle& obj);

auto load_from_prefab(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle;
auto load_from_prefab_bin(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle;

void clone_entity_from_stream(entt::const_handle src_obj, entt::handle& dst_obj);

void save_to_stream(std::ostream& stream, const scene& scn);
void save_to_file(const std::string& absolute_path, const scene& scn);
void save_to_stream_bin(std::ostream& stream, const scene& scn);
void save_to_file_bin(const std::string& absolute_path, const scene& scn);

void load_from_stream(std::istream& stream, scene& scn);
void load_from_file(const std::string& absolute_path, scene& scn);
void load_from_stream_bin(std::istream& stream, scene& scn);
void load_from_file_bin(const std::string& absolute_path, scene& scn);

auto load_from_prefab(const asset_handle<scene_prefab>& pfb, scene& scn) -> bool;
auto load_from_prefab_bin(const asset_handle<scene_prefab>& pfb, scene& scn) -> bool;

void clone_scene_from_stream(const scene& src_scene, scene& dst_scene);
} // namespace ace

namespace ser20
{

SAVE_EXTERN(entt::const_handle);
LOAD_EXTERN(entt::handle);
} // namespace ser20
