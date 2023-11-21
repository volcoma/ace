#include "editing_system.h"

#include <engine/assets/asset_manager.h>

namespace ace
{

void editing_system::select(rttr::variant object)
{
    selection_data.object = object;
}

void editing_system::focus(rttr::variant object)
{
    focused_data.object = object;
}

void editing_system::unselect()
{
    selection_data = {};
    //	imguizmo::enable(false);
    //	imguizmo::enable(true);
}

void editing_system::unfocus()
{
    focused_data = {};
    //	imguizmo::enable(false);
    //	imguizmo::enable(true);
}

void editing_system::close_project()
{
    unselect();
    unfocus();
}
} // namespace ace
