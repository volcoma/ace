#include "thumbnail_manager.h"

#include <engine/assets/asset_manager.h>

#include <engine/animation/animation.h>
#include <engine/audio/audio_clip.h>
#include <engine/ecs/scene.h>
#include <engine/physics/physics_material.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <graphics/texture.h>

#include <filesystem/filesystem.h>
#include <filesystem/watcher.h>

namespace ace
{

template<>
auto thumbnail_manager::get_thumbnail<mesh>(const asset_handle<mesh>& asset) -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.mesh;
}

template<>
auto thumbnail_manager::get_thumbnail<material>(const asset_handle<material>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.material;
}

template<>
auto thumbnail_manager::get_thumbnail<physics_material>(const asset_handle<physics_material>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.physics_material;
}

template<>
auto thumbnail_manager::get_thumbnail<audio_clip>(const asset_handle<audio_clip>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.audio_clip;
}

template<>
auto thumbnail_manager::get_thumbnail<animation>(const asset_handle<animation>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.animation;
}

template<>
auto thumbnail_manager::get_thumbnail<gfx::texture>(const asset_handle<gfx::texture>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }

    return !asset.is_ready() ? thumbnails_.loading : asset;
}

template<>
auto thumbnail_manager::get_thumbnail<gfx::shader>(const asset_handle<gfx::shader>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.shader;
}

template<>
auto thumbnail_manager::get_thumbnail<prefab>(const asset_handle<prefab>& asset) -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.prefab;
}

template<>
auto thumbnail_manager::get_thumbnail<scene_prefab>(const asset_handle<scene_prefab>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.scene_prefab;
}

auto thumbnail_manager::get_thumbnail(const fs::path& path) -> const asset_handle<gfx::texture>&
{
    fs::error_code ec;
    if(fs::is_directory(path, ec))
    {
        return thumbnails_.folder;
    }

    return thumbnails_.file;
}

auto thumbnail_manager::get_icon(const std::string& id) -> const asset_handle<gfx::texture>&
{
    auto it = icons_.find(id);
    if(it == std::end(icons_))
    {
        return thumbnails_.transparent;
    }

    return it->second;
}

auto thumbnail_manager::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& am = ctx.get<asset_manager>();
    thumbnails_.transparent = am.get_asset<gfx::texture>("engine:/data/textures/transparent.png");
    
    thumbnails_.file = am.get_asset<gfx::texture>("editor:/data/icons/file.png");
    thumbnails_.folder = am.get_asset<gfx::texture>("editor:/data/icons/folder.png");
    thumbnails_.folder_empty = am.get_asset<gfx::texture>("editor:/data/icons/folder_empty.png");
    thumbnails_.loading = am.get_asset<gfx::texture>("editor:/data/icons/loading.png");
    thumbnails_.shader = am.get_asset<gfx::texture>("editor:/data/icons/shader.png");
    thumbnails_.material = am.get_asset<gfx::texture>("editor:/data/icons/material.png");
    thumbnails_.physics_material = am.get_asset<gfx::texture>("editor:/data/icons/material.png");
    thumbnails_.mesh = am.get_asset<gfx::texture>("editor:/data/icons/mesh.png");
    thumbnails_.animation = am.get_asset<gfx::texture>("editor:/data/icons/animation.png");
    thumbnails_.prefab = am.get_asset<gfx::texture>("editor:/data/icons/prefab.png");
    thumbnails_.scene_prefab = am.get_asset<gfx::texture>("editor:/data/icons/scene.png");
    thumbnails_.audio_clip = am.get_asset<gfx::texture>("editor:/data/icons/sound.png");

    return true;
}

auto thumbnail_manager::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}
} // namespace ace
