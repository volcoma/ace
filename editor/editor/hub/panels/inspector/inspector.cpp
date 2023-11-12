#include "inspector.h"
#include "inspectors/inspectors.h"


namespace ace
{

namespace
{

} // namespace

void inspector_panel::init(rtti::context& ctx)
{
}

void inspector_panel::draw(rtti::context& ctx)
{
    auto& es = ctx.get<editing_system>();
    auto& selected = es.selection_data.object;

    if(ImGui::BeginMenuBar())
    {
//        if(ImGui::BeginMenu("Options"))
//        {
        bool locked = !!locked_object;

        if (ImGui::MenuItem(locked ? ICON_FA_LOCK : ICON_FA_UNLOCK, nullptr, locked))
        {
            locked = !locked;

            if(locked)
            {
                locked_object = selected;
            }
            else
            {
                locked_object = {};
            }
        }
//        ImGui::EndMenu();
//        }

        ImGui::EndMenuBar();
    }




//    if(ImGui::Checkbox("Lock", &locked))
//    {
//        if(locked)
//        {
//            locked_object = selected;
//        }
//        else
//        {
//            locked_object = {};
//        }
//    }

    if(locked_object)
    {
        inspect_var(ctx, locked_object);
    }
    else if(selected)
    {
        inspect_var(ctx, selected);
    }
}

} // namespace ace
