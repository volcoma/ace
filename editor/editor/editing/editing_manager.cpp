#include "editing_manager.h"

#include <engine/assets/asset_manager.h>

namespace ace
{

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
