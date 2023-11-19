#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class camera_system
{
public:
    auto init(rtti::context& ctx) -> bool;

    void on_frame_update(rtti::context& ctx, delta_t dt);

private:
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
