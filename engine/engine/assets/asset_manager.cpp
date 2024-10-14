#include "asset_manager.h"
#include "impl/asset_reader.h"
#include "impl/importers/mesh_importer.h"

#include <engine/animation/animation.h>
#include <engine/audio/audio_clip.h>
#include <engine/ecs/prefab.h>
#include <engine/meta/assets/asset_database.hpp>
#include <engine/physics/physics_material.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/scripting/script.h>

#include <graphics/shader.h>
#include <graphics/texture.h>

namespace ace
{
asset_manager::asset_manager(rtti::context& ctx) : pool_(*ctx.get<threader>().pool)
{
}

asset_manager::~asset_manager() = default;

void asset_manager::set_parent(asset_manager* parent)
{
    parent_ = parent;
}

auto asset_manager::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    importer::mesh_importer_init();
    {
        auto& storage = add_storage<gfx::shader>();
        storage.load_from_file = asset_reader::load_from_file<gfx::shader>;
        storage.load_from_instance = asset_reader::load_from_instance<gfx::shader>;
    }
    {
        auto& storage = add_storage<gfx::texture>();
        storage.load_from_file = asset_reader::load_from_file<gfx::texture>;
        storage.load_from_instance = asset_reader::load_from_instance<gfx::texture>;
    }

    {
        auto& storage = add_storage<material>();
        storage.load_from_file = asset_reader::load_from_file<material>;
        storage.load_from_instance = asset_reader::load_from_instance<material>;
    }

    {
        auto& storage = add_storage<mesh>();
        storage.load_from_file = asset_reader::load_from_file<mesh>;
        storage.load_from_instance = asset_reader::load_from_instance<mesh>;
    }

    {
        auto& storage = add_storage<animation_clip>();
        storage.load_from_file = asset_reader::load_from_file<animation_clip>;
        storage.load_from_instance = asset_reader::load_from_instance<animation_clip>;
    }

    {
        auto& storage = add_storage<prefab>();
        storage.load_from_file = asset_reader::load_from_file<prefab>;
        storage.load_from_instance = asset_reader::load_from_instance<prefab>;
    }

    {
        auto& storage = add_storage<scene_prefab>();
        storage.load_from_file = asset_reader::load_from_file<scene_prefab>;
        storage.load_from_instance = asset_reader::load_from_instance<scene_prefab>;
    }

    {
        auto& storage = add_storage<physics_material>();
        storage.load_from_file = asset_reader::load_from_file<physics_material>;
        storage.load_from_instance = asset_reader::load_from_instance<physics_material>;
    }

    {
        auto& storage = add_storage<audio_clip>();
        storage.load_from_file = asset_reader::load_from_file<audio_clip>;
        storage.load_from_instance = asset_reader::load_from_instance<audio_clip>;
    }

    {
        auto& storage = add_storage<script>();
        storage.load_from_file = asset_reader::load_from_file<script>;
        storage.load_from_instance = asset_reader::load_from_instance<script>;
    }

    return true;
}

auto asset_manager::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void asset_manager::unload_all()
{
    for(auto& pair : storages_)
    {
        auto& storage = pair.second;
        storage->unload_all(pool_);
    }

    {
        std::lock_guard<std::mutex> lock(db_mutex_);
        databases_.clear();
    }
}

void asset_manager::unload_group(const std::string& group)
{
    for(auto& pair : storages_)
    {
        auto& storage = pair.second;
        storage->unload_group(pool_, group);
    }

    {
        remove_database(group);
    }
}

auto asset_manager::get_database(const std::string& key) -> asset_database&
{
    auto protocol = fs::extract_protocol(fs::path(key));
    return databases_[protocol.generic_string()];
}

void asset_manager::remove_database(const std::string& key)
{
    auto protocol = fs::extract_protocol(fs::path(key));

    std::lock_guard<std::mutex> lock(db_mutex_);
    databases_.erase(protocol.generic_string());
}

auto asset_manager::load_database(const std::string& protocol) -> bool
{
    auto assets_pack = fs::resolve_protocol(protocol + "assets.pack");

    std::lock_guard<std::mutex> lock(db_mutex_);
    auto& db = get_database(protocol);
    return load_from_file(assets_pack.string(), db);
}

void asset_manager::save_database(const std::string& protocol, const fs::path& path)
{
    std::lock_guard<std::mutex> lock(db_mutex_);
    auto& db = get_database(protocol);
    save_to_file(path.string(), db);
}

auto asset_manager::add_asset(const std::string& key) -> hpp::uuid
{
    asset_meta meta;
    meta.type = fs::path(key).extension().string();

    if(meta.type.empty())
    {
        meta.uid = generate_uuid(key);
    }
    else
    {
        meta.uid = generate_uuid();
    }
    return add_asset_info_for_key(key, meta);
}

auto asset_manager::add_asset_info_for_path(const fs::path& path, const asset_meta& meta) -> hpp::uuid
{
    auto key = fs::convert_to_protocol(path).generic_string();
    return add_asset_info_for_key(key, meta);
}

auto asset_manager::add_asset_info_for_key(const std::string& key, const asset_meta& meta) -> hpp::uuid
{
    std::lock_guard<std::mutex> lock(db_mutex_);
    auto& db = get_database(key);
    return db.add_asset(key, meta);
}

auto asset_manager::get_metadata(const hpp::uuid& uid) -> asset_database::meta
{
    std::lock_guard<std::mutex> lock(db_mutex_);
    for(auto& kvp : databases_)
    {
        auto& db = kvp.second;
        const auto& meta = db.get_metadata(uid);
        if(!meta.location.empty())
        {
            return meta;
        }
    }
    return {};
}

void asset_manager::remove_asset_info_for_path(const fs::path& path)
{
    auto key = fs::convert_to_protocol(path).generic_string();
    remove_asset_info_for_key(key);
}

void asset_manager::remove_asset_info_for_key(const std::string& key)
{
    std::lock_guard<std::mutex> lock(db_mutex_);
    auto& db = get_database(key);
    db.remove_asset(key);
}

} // namespace ace
