#include "editing_system.h"

namespace ace
{

editing_system::editing_system()
{
}

void editing_system::save_editor_camera()
{
}

void editing_system::load_editor_camera()
{
}

void editing_system::select(rttr::variant object)
{
    selection_data.object = object;
}

void editing_system::unselect()
{
    selection_data = {};
    //	imguizmo::enable(false);
    //	imguizmo::enable(true);
}

void editing_system::close_project()
{
    save_editor_camera();
    unselect();
    scene.clear();
}
} // namespace ace
