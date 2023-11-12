#pragma once
#include "../../../imgui/integration/imgui.h"
#include <filesystem/cache.hpp>

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class content_browser
{
public:
    void init(rtti::context& ctx);

    void draw(rtti::context& ctx);

private:
    void draw_details(rtti::context& ctx, const fs::path& root_path);

    void draw_as_explorer(rtti::context& ctx, const fs::path& root_path);
    void context_menu(rtti::context& ctx);
    void context_create_menu(rtti::context& ctx);
    void set_cache_path(const fs::path& path);
    void import(rtti::context& ctx);
    void on_import(rtti::context& ctx, const std::vector<std::string>& paths);

    fs::directory_cache cache_;
    fs::recursive_directory_cache recursive_cache_;

    fs::path cache_path_with_protocol_;
    fs::path root_;
    float scale_ = 0.75f;

    struct icons_cache
    {
        asset_handle<gfx::texture> folder;
        asset_handle<gfx::texture> folder_empty;
        asset_handle<gfx::texture> loading;
        asset_handle<gfx::texture> shader;
        asset_handle<gfx::texture> material;

    } icons;
};
} // namespace ace
