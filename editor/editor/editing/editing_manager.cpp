#include "editing_manager.h"

namespace ace
{
auto editing_manager::init(rtti::context& ctx) -> bool
{
    return true;
}

auto editing_manager::deinit(rtti::context& ctx) -> bool
{
    unselect();
    unfocus();
    return true;
}

void editing_manager::select(rttr::variant object)
{
    selection_data.object = object;
}

void editing_manager::focus(rttr::variant object)
{
    focused_data.object = object;
}

void editing_manager::unselect()
{
    selection_data = {};
    //	imguizmo::enable(false);
    //	imguizmo::enable(true);
}

void editing_manager::unfocus()
{
    focused_data = {};
    //	imguizmo::enable(false);
    //	imguizmo::enable(true);
}

void editing_manager::close_project()
{
    unselect();
    unfocus();
}
} // namespace ace
