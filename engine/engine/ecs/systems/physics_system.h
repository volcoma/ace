#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class physics_system
{
public:
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
};
} // namespace ace
