#include "gizmos.h"
#include <engine/rendering/camera.h>
#include "gizmo_entity.h"
#include "gizmo_physics_component.h"

namespace ace
{

gizmo_registry::gizmo_registry()
{
    auto inspector_types = rttr::type::get<gizmo>().get_derived_classes();
    for(auto& inspector_type : inspector_types)
    {
        auto inspected_type_var = inspector_type.get_metadata("inspected_type");
        if(inspected_type_var)
        {
            auto inspected_type = inspected_type_var.get_value<rttr::type>();
            auto inspector_var = inspector_type.create();
            if(inspector_var)
            {
                type_map[inspected_type] = inspector_var.get_value<std::shared_ptr<gizmo>>();
            }
        }
    }
}


auto get_gizmo(rtti::context& ctx, rttr::type type) -> std::shared_ptr<gizmo>
{
    auto& registry = ctx.get<gizmo_registry>();
    return registry.type_map[type];
}


void draw_gizmo_var(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd)
{
    rttr::instance object = var;
    auto type = object.get_derived_type();

    auto giz = get_gizmo(ctx, type);
    if(giz)
    {
        giz->draw(ctx, var, cam, dd);
    }
}
} // namespace ace
