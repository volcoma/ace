#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/debugdraw.h>
#include <reflection/reflection.h>
#include <reflection/registration.h>

namespace ace
{
class camera;


struct gizmo
{
    REFLECTABLEV(gizmo)

    virtual ~gizmo() = default;

    virtual void draw(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd) = 0;
};

REFLECT_INLINE(gizmo)
{
    rttr::registration::class_<gizmo>("gizmo");
}
#define GIZMO_REFLECT(gizmo_renderer_type, inspected_type)                                                             \
    REFLECT_INLINE(gizmo_renderer_type)                                                                                \
    {                                                                                                                  \
        rttr::registration::class_<gizmo_renderer_type>(#gizmo_renderer_type)(                                         \
            rttr::metadata("inspected_type", rttr::type::get<inspected_type>()))                                       \
            .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);                                                   \
    }

#define DECLARE_GIZMO(gizmo_renderer_type, inspected_type)                                                             \
    struct gizmo_renderer_type : public gizmo                                                                          \
    {                                                                                                                  \
        REFLECTABLEV(gizmo_renderer_type, gizmo)                                                                       \
        void draw(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd);                                           \
    };                                                                                                                 \
    GIZMO_REFLECT(gizmo_renderer_type, inspected_type)

} // namespace ace
