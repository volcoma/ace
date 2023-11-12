#include "asset_manager.h"
#include "impl/asset_reader.h"
#include "impl/asset_writer.h"

#include <graphics/shader.h>
#include <graphics/texture.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>

namespace ace
{
asset_manager::asset_manager(rtti::context& ctx)
    : pool_(*ctx.get<threader>().pool)
{
}

asset_manager::~asset_manager() = default;


auto asset_manager::init() -> bool
{
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
