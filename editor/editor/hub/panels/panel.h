#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/texture.h>

#include "console_log/console_log.h"
#include "content_browser/content_browser.h"
#include "hierarchy_graph/hierarchy_graph.h"
#include "scene_panel/scene_panel.h"
#include "inspector/inspector.h"

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
    std::shared_ptr<console_log> console_log_;
    std::unique_ptr<content_browser> content_browser_;
    std::unique_ptr<hierarchy_graph> hierarchy_graph_;
    std::unique_ptr<inspector_panel> inspector_;
    std::unique_ptr<scene_panel> scene_panel_;

};
} // namespace ace
