#pragma once
#include <editor/imgui/integration/imgui.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class scene_panel
{
public:
    void init(rtti::context& ctx);

    void draw(rtti::context& ctx);

private:
    void draw_menubar(rtti::context& ctx);

    bool show_statistics_{};
    bool show_gbuffer_{};
    bool enable_profiler_{};

};
} // namespace ace
