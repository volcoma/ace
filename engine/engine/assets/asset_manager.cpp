#include "asset_manager.h"
#include "impl/asset_reader.h"

#include <graphics/shader.h>
#include <graphics/texture.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/animation/animation.h>
#include <engine/ecs/prefab.h>
#include <engine/physics/physics_material.h>

namespace ace
{
asset_manager::asset_manager(rtti::context& ctx)
    : pool_(*ctx.get<threader>().pool)
{
}

asset_manager::~asset_manager() = default;


auto asset_manager::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

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
        auto& storage = add_storage<animation>();
        storage.load_from_file = asset_reader::load_from_file<animation>;
        storage.load_from_instance = asset_reader::load_from_instance<animation>;
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
}

void asset_manager::unload_group(const std::string& group)
{
	for(auto& pair : storages_)
	{
		auto& storage = pair.second;
		storage->unload_group(pool_, group);
	}
}
}
