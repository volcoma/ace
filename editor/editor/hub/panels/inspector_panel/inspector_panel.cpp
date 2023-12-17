#include "inspector_panel.h"
#include "inspectors/inspectors.h"

#include <editor/imgui/integration/imgui.h>
#include <editor/editing/editing_manager.h>


namespace ace
{

namespace
{

} // namespace

void inspector_panel::init(rtti::context& ctx)
{
    ctx.add<inspector_registry>();
}

void inspector_panel::deinit(rtti::context& ctx)
{
    ctx.remove<inspector_registry>();
}

void inspector_panel::draw(rtti::context& ctx)
{
    auto& em = ctx.get<editing_manager>();
    auto& selected = em.selection_data.object;

    if(ImGui::BeginMenuBar())
    {
        bool locked = !!locked_object_;

        if(ImGui::MenuItem(locked ? ICON_MDI_LOCK : ICON_MDI_LOCK_OPEN_VARIANT, nullptr, locked))
        {
            locked = !locked;

            if(locked)
            {
                locked_object_ = selected;
            }
            else
            {
                locked_object_ = {};
            }
        }

        ImGui::SetItemTooltip("%s", "Lock/Unlock Inspector");


        ImGui::EndMenuBar();
    }

    if(locked_object_)
    {
        inspect_var(ctx, locked_object_);
    }
    else if(selected)
    {
        inspect_var(ctx, selected);
    }
}

} // namespace ace
