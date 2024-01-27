#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/texture.h>

#include "console_log_panel/console_log_panel.h"
#include "content_browser_panel/content_browser_panel.h"
#include "hierarchy_panel/hierarchy_panel.h"
#include "scene_panel/scene_panel.h"
#include "game_panel/game_panel.h"
#include "inspector_panel/inspector_panel.h"
#include "statistics_panel/statistics_panel.h"

namespace ace
{

class imgui_panels
{
public:
    imgui_panels();
    ~imgui_panels();

    void init(rtti::context& ctx);
    void deinit(rtti::context& ctx);

    void setup_panels(rtti::context& ctx, ImGuiID dockspace_id);

    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);
    void on_frame_ui_render(rtti::context& ctx);

    void draw_panels(rtti::context& ctx);
    void draw_menubar(rtti::context& ctx);
    void draw_footer(rtti::context& ctx);
    void set_dark_theme();
    void set_photoshop_theme();
private:
    std::shared_ptr<console_log_panel> console_log_panel_;
    std::unique_ptr<content_browser_panel> content_browser_panel_;
    std::unique_ptr<hierarchy_panel> hierarchy_panel_;
    std::unique_ptr<inspector_panel> inspector_panel_;
    std::unique_ptr<scene_panel> scene_panel_;
    std::unique_ptr<game_panel> game_panel_;
    std::unique_ptr<statistics_panel> statistics_panel_;

    bool is_playing_{};

};
} // namespace ace
