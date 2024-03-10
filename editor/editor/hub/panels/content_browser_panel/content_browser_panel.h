#pragma once
#include <editor/imgui/integration/imgui.h>
#include <filesystem/cache.hpp>

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class content_browser_panel
{
public:
    void init(rtti::context& ctx);
    void on_frame_ui_render(rtti::context& ctx);

private:
    void draw(rtti::context& ctx);
    void draw_details(rtti::context& ctx, const fs::path& root_path);

    void draw_as_explorer(rtti::context& ctx, const fs::path& root_path);
    void context_menu(rtti::context& ctx);
    void context_create_menu(rtti::context& ctx);
    void set_cache_path(const fs::path& path);
    void import(rtti::context& ctx);
    void on_import(rtti::context& ctx, const std::vector<std::string>& paths);

    fs::directory_cache cache_;

    fs::path root_;
    int refresh_{};
    float scale_ = 0.65f;
};
} // namespace ace
