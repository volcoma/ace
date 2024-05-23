#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/frame_buffer.h>
#include <graphics/shader.h>
#include <graphics/texture.h>

#include <engine/assets/asset_handle.h>
#include <engine/ecs/scene.h>

namespace ace
{
struct thumbnail_manager
{
    struct generated_thumbnail
    {
        auto get() -> gfx::texture::ptr;
        void set(gfx::frame_buffer::ptr fbo);

        bool needs_regeneration{};
        gfx::frame_buffer::ptr thumbnail;
    };

    struct generator
    {
        std::map<hpp::uuid, generated_thumbnail> thumbnails;

        auto get_scene() -> scene&
        {
            reset_wait();
            remaining--;
            return scenes[remaining];
        }

        void reset()
        {
            if(wait_frames-- <= 0)
            {
                for(auto& scn : scenes)
                {
                    scn.unload();
                }
                remaining = scenes.size();

                reset_wait();
            }
        }

        void reset_wait()
        {
            wait_frames = 2;
        }

        int remaining{0};

        std::array<scene, 3> scenes;

        int wait_frames{};
    };

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;
    void on_frame_update(rtti::context& ctx, delta_t);

    template<typename T>
    auto get_thumbnail(const asset_handle<T>& asset) -> gfx::texture::ptr;

    auto get_thumbnail(const fs::path& path) -> gfx::texture::ptr;

    auto get_icon(const std::string& id) -> asset_handle<gfx::texture>;

    void regenerate_thumbnail(const hpp::uuid& uid);
    void remove_thumbnail(const hpp::uuid& uid);
    void clear_thumbnails();

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


    generator gen_;

    std::map<std::string, asset_handle<gfx::texture>> icons_;
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
