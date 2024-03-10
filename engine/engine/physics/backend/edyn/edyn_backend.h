#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/physics/ecs/components/physics_component.h>

namespace ace
{
struct edyn_backend
{
    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_play_begin(rtti::context& ctx);
    void on_play_end(rtti::context& ctx);
    void on_pause(rtti::context& ctx);
    void on_resume(rtti::context& ctx);
    void on_skip_next_frame(rtti::context& ctx);

    void on_apply_impulse(physics_component& comp, const math::vec3& impulse);
    void on_apply_torque_impulse(physics_component& comp, const math::vec3& impulse);
    void on_clear_kinematic_velocities(physics_component& comp);
};
} // namespace ace
