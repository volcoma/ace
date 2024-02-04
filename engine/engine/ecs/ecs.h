#pragma once
#include "scene.h"

namespace ace
{

struct ecs
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void on_frame_render(rtti::context& ctx, delta_t dt);

    void unload_scene();
    auto get_scene() -> scene&;

private:
    scene scene_{};
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};

} // namespace ace
