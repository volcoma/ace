#include "thumbnail_manager.h"

#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/audio/audio_clip.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>

#include <engine/ecs/scene.h>
#include <engine/ecs/systems/rendering_path.h>
#include <engine/engine.h>
#include <engine/physics/physics_material.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <graphics/render_pass.h>
#include <graphics/texture.h>

#include <filesystem/filesystem.h>
#include <filesystem/watcher.h>

namespace ace
{

namespace
{

template<typename T>
auto make_thumbnail(std::map<hpp::uuid, thumbnail_manager::generated_thumbnail>& cache, const asset_handle<T>& asset, bool recreate)
    -> gfx::frame_buffer::ptr
{
    auto& thumbnail = cache[asset.uid()];
    auto current_fbo = thumbnail.get();

    if(recreate)
    {
        scene scn;
        auto& ctx = engine::context();
        auto& def = ctx.get<defaults>();
        def.create_default_3d_scene_for_asset_preview(ctx, scn, asset);

        delta_t dt(0.016667f);

        auto& rpath = ctx.get<rendering_path>();
        rpath.prepare_scene(scn, dt);
        auto new_fbo = rpath.render_scene(scn, dt);
        thumbnail.set(new_fbo);
    }

    return current_fbo;
}
} // namespace

template<>
auto thumbnail_manager::get_thumbnail<mesh>(const asset_handle<mesh>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }
    return !asset.is_ready() ? thumbnails_.loading.get() : thumbnails_.mesh.get();
}

template<>
auto thumbnail_manager::get_thumbnail<material>(const asset_handle<material>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }

    if(!asset.is_ready())
    {
        return thumbnails_.loading.get();
    }

    auto gen_thumb = make_thumbnail(generated_thumbnails_, asset, should_regenerate(asset.uid()));

    if(gen_thumb)
    {
        const auto& att = gen_thumb->get_attachment();
        if(att.texture)
        {
            return *att.texture;
        }
    }

    return thumbnails_.material.get();
}

template<>
auto thumbnail_manager::get_thumbnail<physics_material>(const asset_handle<physics_material>& asset)
    -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }
    return !asset.is_ready() ? thumbnails_.loading.get() : thumbnails_.physics_material.get();
}

template<>
auto thumbnail_manager::get_thumbnail<audio_clip>(const asset_handle<audio_clip>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }
    return !asset.is_ready() ? thumbnails_.loading.get() : thumbnails_.audio_clip.get();
}

template<>
auto thumbnail_manager::get_thumbnail<animation>(const asset_handle<animation>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }
    return !asset.is_ready() ? thumbnails_.loading.get() : thumbnails_.animation.get();
}

template<>
auto thumbnail_manager::get_thumbnail<gfx::texture>(const asset_handle<gfx::texture>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }

    return !asset.is_ready() ? thumbnails_.loading.get() : asset.get();
}

template<>
auto thumbnail_manager::get_thumbnail<gfx::shader>(const asset_handle<gfx::shader>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }
    return !asset.is_ready() ? thumbnails_.loading.get() : thumbnails_.shader.get();
}

template<>
auto thumbnail_manager::get_thumbnail<prefab>(const asset_handle<prefab>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }

    if(!asset.is_ready())
    {
        return thumbnails_.loading.get();
    }

    auto gen_thumb = make_thumbnail(generated_thumbnails_, asset, should_regenerate(asset.uid()));

    if(gen_thumb)
    {
        const auto& att = gen_thumb->get_attachment();
        if(att.texture)
        {
            return *att.texture;
        }
    }

    return thumbnails_.prefab.get();
}

template<>
auto thumbnail_manager::get_thumbnail<scene_prefab>(const asset_handle<scene_prefab>& asset) -> const gfx::texture&
{
    if(!asset.is_valid())
    {
        return thumbnails_.transparent.get();
    }
    return !asset.is_ready() ? thumbnails_.loading.get() : thumbnails_.scene_prefab.get();
}

auto thumbnail_manager::get_thumbnail(const fs::path& path) -> const gfx::texture&
{
    fs::error_code ec;
    if(fs::is_directory(path, ec))
    {
        return thumbnails_.folder.get();
    }

    return thumbnails_.file.get();
}

auto thumbnail_manager::get_icon(const std::string& id) -> asset_handle<gfx::texture>
{
    auto it = icons_.find(id);
    if(it == std::end(icons_))
    {
        return thumbnails_.transparent;
    }

    return it->second;
}

void thumbnail_manager::regenerate_thumbnail(const hpp::uuid& uid)
{
    needs_regeneration_.emplace(uid);
}

auto thumbnail_manager::should_regenerate(const hpp::uuid& uid) const -> bool
{
    auto it = needs_regeneration_.find(uid);
    bool recreate = it != std::end(needs_regeneration_);
    if(recreate)
    {
        needs_regeneration_.erase(it);
    }

    return recreate;
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
