#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/physics/backend/bullet/bullet_backend.h>

namespace ace
{
class physics_system
{
public:
    using backend_type = bullet_backend;

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    static void on_create_component(entt::registry& r, const entt::entity e);
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    static void apply_impulse(physics_component& comp, const math::vec3& impulse);
    static void apply_torque_impulse(physics_component& comp, const math::vec3& torque_impulse);
    static void clear_kinematic_velocities(physics_component& comp);

private:
    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_play_begin(rtti::context& ctx);
    void on_play_end(rtti::context& ctx);
    void on_pause(rtti::context& ctx);
    void on_resume(rtti::context& ctx);
    void on_skip_next_frame(rtti::context& ctx);

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

    backend_type backend_;
};
} // namespace ace
