#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/texture.h>
#include <graphics/shader.h>

#include <engine/assets/asset_handle.h>

namespace ace
{
struct thumbnail_manager
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    template<typename T>
    auto get_thumbnail(const asset_handle<T>& asset) -> const asset_handle<gfx::texture>&;

    auto get_thumbnail(const fs::path& path) -> const asset_handle<gfx::texture>&;

    auto get_icon(const std::string& id) -> const asset_handle<gfx::texture>&;

private:
    struct thumbnail_cache
    {
        asset_handle<gfx::texture> transparent;
        asset_handle<gfx::texture> folder;
        asset_handle<gfx::texture> folder_empty;
        asset_handle<gfx::texture> file;
        asset_handle<gfx::texture> loading;
        asset_handle<gfx::texture> shader;
        asset_handle<gfx::texture> material;
        asset_handle<gfx::texture> mesh;
        asset_handle<gfx::texture> animation;

    } thumbnails_;

    std::map<std::string, asset_handle<gfx::texture>> icons_;
};
} // namespace ace
