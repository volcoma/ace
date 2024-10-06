#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/texture.h>

#include "console_log_panel/console_log_panel.h"
#include "content_browser_panel/content_browser_panel.h"
#include "deploy_panel/deploy_panel.h"
#include "dockspace.h"
#include "footer_panel/footer_panel.h"
#include "game_panel/game_panel.h"
#include "header_panel/header_panel.h"
#include "hierarchy_panel/hierarchy_panel.h"
#include "inspector_panel/inspector_panel.h"
#include "scene_panel/scene_panel.h"
#include "statistics_panel/statistics_panel.h"
#include "animation_panel/animation_panel.h"

namespace ace
{

class imgui_panels
{
public:
    imgui_panels();
    ~imgui_panels();

    void init(rtti::context& ctx);
    void deinit(rtti::context& ctx);

    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);
    void on_frame_ui_render(rtti::context& ctx);

    void set_dark_theme();
    void set_photoshop_theme();
    void set_dark_theme2();

    deploy_panel& get_deploy_panel()
    {
        return *deploy_panel_;
    }

    scene_panel& get_scene_panel()
    {
        return *scene_panel_;
    }
private:
    std::shared_ptr<console_log_panel> console_log_panel_;
    std::unique_ptr<content_browser_panel> content_browser_panel_;
    std::unique_ptr<hierarchy_panel> hierarchy_panel_;
    std::unique_ptr<inspector_panel> inspector_panel_;
    std::unique_ptr<scene_panel> scene_panel_;
    std::unique_ptr<game_panel> game_panel_;
    std::unique_ptr<statistics_panel> statistics_panel_;
    std::unique_ptr<header_panel> header_panel_;
    std::unique_ptr<footer_panel> footer_panel_;
    std::unique_ptr<deploy_panel> deploy_panel_;
    std::unique_ptr<animation_panel> animation_panel_;

    std::unique_ptr<dockspace> cenral_dockspace_;
};
} // namespace ace
