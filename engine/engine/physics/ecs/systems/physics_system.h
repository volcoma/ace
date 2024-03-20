#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/physics/backend/edyn/edyn_backend.h>
#include <engine/physics/backend/bullet/bullet_backend.h>

namespace ace
{
class physics_system
{
public:
    using backend_type = bullet_backend;

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

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
