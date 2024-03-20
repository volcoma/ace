#include "physics_system.h"
#include <engine/defaults/defaults.h>
#include <engine/events.h>

#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/physics/ecs/components/physics_component.h>

#include <edyn/edyn.hpp>
#include <logging/logging.h>

namespace ace
{

auto physics_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();
    ev.on_frame_update.connect(sentinel_, this, &physics_system::on_frame_update);

    ev.on_play_begin.connect(sentinel_, -100, this, &physics_system::on_play_begin);
    ev.on_play_end.connect(sentinel_, 100, this, &physics_system::on_play_end);
    ev.on_pause.connect(sentinel_, -100, this, &physics_system::on_pause);
    ev.on_resume.connect(sentinel_, 100, this, &physics_system::on_resume);
    ev.on_skip_next_frame.connect(sentinel_, -100, this, &physics_system::on_skip_next_frame);

    return true;
}

auto physics_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void physics_system::on_play_begin(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;
    auto& emitter = registry.ctx().emplace<physics_component_emitter>();

    emitter.on_apply_impulse().connect<&backend_type::on_apply_impulse>(&backend_);
    emitter.on_apply_torque_impulse().connect<&backend_type::on_apply_torque_impulse>(&backend_);

    backend_.on_play_begin(ctx);
}

void physics_system::on_play_end(rtti::context& ctx)
{
    backend_.on_play_end(ctx);

    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    registry.ctx().erase<physics_component_emitter>();
}

void physics_system::on_pause(rtti::context& ctx)
{
    backend_.on_pause(ctx);
}

void physics_system::on_resume(rtti::context& ctx)
{
    backend_.on_resume(ctx);
}

void physics_system::on_skip_next_frame(rtti::context& ctx)
{
    backend_.on_skip_next_frame(ctx);
}

void physics_system::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& ev = ctx.get<events>();

    if(ev.is_playing && !ev.is_paused)
    {
        backend_.on_frame_update(ctx, dt);
    }
}

} // namespace ace
