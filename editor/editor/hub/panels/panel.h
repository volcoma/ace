#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/texture.h>

#include "console_log_panel/console_log_panel.h"
#include "content_browser_panel/content_browser_panel.h"
#include "hierarchy_panel/hierarchy_panel.h"
#include "scene_panel/scene_panel.h"
#include "inspector_panel/inspector_panel.h"

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

    void draw(rtti::context& ctx);

    void draw_panels(rtti::context& ctx);
    void set_dark_theme();
    void set_photoshop_theme();
private:
    std::shared_ptr<console_log_panel> console_log_panel_;
    std::unique_ptr<content_browser_panel> content_browser_panel_;
    std::unique_ptr<hierarchy_panel> hierarchy_panel_;
    std::unique_ptr<inspector_panel> inspector_panel_;
    std::unique_ptr<scene_panel> scene_panel_;

};
} // namespace ace
