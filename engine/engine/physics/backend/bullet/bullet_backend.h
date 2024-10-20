#pragma once
#include <engine/engine_export.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/physics/ecs/components/physics_component.h>
#include <engine/rendering/camera.h>
#include <graphics/debugdraw.h>

namespace ace
{
class camera;


struct bullet_backend
{
    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_play_begin(rtti::context& ctx);
    void on_play_end(rtti::context& ctx);
    void on_pause(rtti::context& ctx);
    void on_resume(rtti::context& ctx);
    void on_skip_next_frame(rtti::context& ctx);

    static void apply_impulse(physics_component& comp, const math::vec3& impulse);
    static void apply_torque_impulse(physics_component& comp, const math::vec3& impulse);
    static void clear_kinematic_velocities(physics_component& comp);

    static void on_create_component(entt::registry& r, const entt::entity e);
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    static void draw_system_gizmos(rtti::context& ctx, const camera& cam, gfx::dd_raii& dd);
    static void draw_gizmo(rtti::context& ctx, physics_component& comp, const camera& cam, gfx::dd_raii& dd);
};
} // namespace ace
