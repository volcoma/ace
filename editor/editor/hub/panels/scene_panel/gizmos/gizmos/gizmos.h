#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <reflection/reflection.h>
#include <reflection/registration.h>
#include "gizmo.h"

namespace ace
{

struct gizmo_registry
{
    gizmo_registry();

    std::unordered_map<rttr::type, std::shared_ptr<gizmo>> type_map;
};

void draw_gizmo_var(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd);

template<typename T>
void draw_gizmo(rtti::context& ctx, T* obj, const camera& cam, gfx::dd_raii& dd)
{
    rttr::variant var = obj;
    draw_gizmo_var(ctx, var, cam, dd);
}

template<typename T>
void draw_gizmo(rtti::context& ctx, T& obj, const camera& cam, gfx::dd_raii& dd)
{
    draw_gizmo(ctx, &obj, cam, dd);
}

} // namespace ace
