#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/frame_buffer.h>
#include <graphics/shader.h>
#include <graphics/texture.h>

#include <engine/assets/asset_handle.h>

namespace ace
{
struct thumbnail_manager
{

    struct generated_thumbnail
    {
        auto get() -> gfx::frame_buffer::ptr
        {
            if(fbos[1] && gfx::get_render_frame() >= frame_set + 2)
            {
                std::swap(fbos[0], fbos[1]);
                fbos[1].reset();
            }

            return fbos[0];
        }
        void set(gfx::frame_buffer::ptr fbo)
        {
            frame_set = gfx::get_render_frame();
            fbos[1] = fbo;
        }

        std::array<gfx::frame_buffer::ptr, 2> fbos;
        uint32_t frame_set{};
    };

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    template<typename T>
    auto get_thumbnail(const asset_handle<T>& asset) -> const gfx::texture&;

    auto get_thumbnail(const fs::path& path) -> const gfx::texture&;

    auto get_icon(const std::string& id) -> asset_handle<gfx::texture>;

    void regenerate_thumbnail(const hpp::uuid& uid);

    auto should_regenerate(const hpp::uuid& uid) const -> bool;

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
        asset_handle<gfx::texture> physics_material;
        asset_handle<gfx::texture> mesh;
        asset_handle<gfx::texture> animation;
        asset_handle<gfx::texture> audio_clip;
        asset_handle<gfx::texture> prefab;
        asset_handle<gfx::texture> scene_prefab;

    } thumbnails_;


    std::map<hpp::uuid, generated_thumbnail> generated_thumbnails_;

    std::map<std::string, asset_handle<gfx::texture>> icons_;

    mutable std::set<hpp::uuid> needs_regeneration_;
};
} // namespace ace
