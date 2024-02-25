#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{

class runner
{
public:
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

private:
    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
