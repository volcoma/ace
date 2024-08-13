#pragma once
#include <editor/imgui/integration/imgui.h>
#include <filesystem/cache.hpp>

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{

struct content_browser_item
{
    using on_action_t = std::function<void()>;
    using on_rename_t = std::function<void(const std::string&)>;

    content_browser_item(const fs::directory_cache::cache_entry& e)
        : entry(e)
    {

    }

    const fs::directory_cache::cache_entry& entry;
    on_action_t on_click;
    on_action_t on_double_click;
    on_action_t on_delete;
    on_rename_t on_rename;

    gfx::texture::ptr icon;
    bool is_loading{};
    bool is_selected{};
    bool is_focused{};
    float size{};
};


class content_browser_panel
{
public:
    void init(rtti::context& ctx);
    void deinit(rtti::context& ctx);

    void on_frame_ui_render(rtti::context& ctx, const char* name);
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

    ImGuiTextFilter filter_;
    fs::path root_;
    int refresh_{};
    float scale_ = 0.65f;

    int focus_frames_{};
};
} // namespace ace
