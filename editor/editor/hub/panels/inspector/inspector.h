#pragma once
#include <editor/imgui/integration/imgui.h>
#include <editor/editing/editing_system.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class inspector_panel
{
public:
    void init(rtti::context& ctx);

    void draw(rtti::context& ctx);

private:


    struct icons_cache
    {
//        asset_handle<gfx::texture> folder;
//        asset_handle<gfx::texture> folder_empty;
//        asset_handle<gfx::texture> loading;
//        asset_handle<gfx::texture> shader;

    } icons;

    rttr::variant locked_object;
};
} // namespace ace
