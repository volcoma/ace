#include "thumbnail_system.h"

#include <engine/assets/asset_manager.h>

// namespace gfx
//{
// struct texture;
// struct shader;
// }

namespace ace
{
class mesh;
class material;
struct animation;

template<>
auto thumbnail_system::get_thumbnail<mesh>(const asset_handle<mesh>& asset) -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.mesh;
}

template<>
auto thumbnail_system::get_thumbnail<material>(const asset_handle<material>& asset) -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.material;
}

template<>
auto thumbnail_system::get_thumbnail<animation>(const asset_handle<animation>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.animation;
}

template<>
auto thumbnail_system::get_thumbnail<gfx::texture>(const asset_handle<gfx::texture>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }

    return !asset.is_ready() ? thumbnails_.loading : asset;
}

template<>
auto thumbnail_system::get_thumbnail<gfx::shader>(const asset_handle<gfx::shader>& asset)
    -> const asset_handle<gfx::texture>&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent;
    }
    return !asset.is_ready() ? thumbnails_.loading : thumbnails_.shader;
}

auto thumbnail_system::get_thumbnail(const fs::path& path) -> const asset_handle<gfx::texture>&
{
    fs::error_code ec;
    if(fs::is_empty(path, ec))
    {
        return thumbnails_.folder_empty;
    }

    return thumbnails_.folder;
}

auto thumbnail_system::init(rtti::context& ctx) -> bool
{
    auto& am = ctx.get<asset_manager>();
    thumbnails_.transparent = am.load<gfx::texture>("engine:/data/textures/transparent.png");

    thumbnails_.folder = am.load<gfx::texture>("editor:/data/icons/folder.png");
    thumbnails_.folder_empty = am.load<gfx::texture>("editor:/data/icons/folder_empty.png");
    thumbnails_.loading = am.load<gfx::texture>("editor:/data/icons/loading.png");
    thumbnails_.shader = am.load<gfx::texture>("editor:/data/icons/shader.png");
    thumbnails_.material = am.load<gfx::texture>("editor:/data/icons/material.png");
    thumbnails_.mesh = am.load<gfx::texture>("editor:/data/icons/mesh.png");
    thumbnails_.animation = am.load<gfx::texture>("editor:/data/icons/animation.png");

    return true;
}

} // namespace ace
