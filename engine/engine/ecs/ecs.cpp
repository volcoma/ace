#include "ecs.h"

#include <engine/events.h>
#include <logging/logging.h>

namespace ace
{

auto ecs::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto ecs::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    unload_scene();

    return true;
}

void ecs::unload_scene()
{
    scene_.unload();
}

auto ecs::get_scene() -> scene&
{
    return scene_;
}

auto ecs::get_scene() const -> const scene&
{
    return scene_;
}

} // namespace ace
