#include "physics_system.h"
#include <engine/events.h>
#include <engine/defaults/defaults.h>

#include <engine/ecs/ecs.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/components/rigidbody_component.h>
#include <engine/ecs/components/box_collider_component.h>

#include <logging/logging.h>
#include <edyn/edyn.hpp>

namespace ace
{

namespace
{
const uint8_t system_id = 1;

void to_physics(transform_component& transform, edyn::position& epos, edyn::orientation& eorientation)
{
    if(transform.is_dirty(system_id))
    {
        auto e = transform.get_owner();

        auto p = transform.get_position_global();
        epos.x = p.x;
        epos.y = p.y;
        epos.z = p.z;
        e.patch<edyn::position>();

        auto q = transform.get_rotation_global();
        eorientation.x = q.x;
        eorientation.y = q.y;
        eorientation.z = q.z;
        eorientation.w = q.w;
        e.patch<edyn::orientation>();

        edyn::wake_up_entity(*e.registry(), e.entity());
    }
}

void from_physics(transform_component& transform, edyn::present_position& epos, edyn::present_orientation& eorientation)
{
    math::vec3 p;
    p.x = epos.x;
    p.y = epos.y;
    p.z = epos.z;
    transform.set_position_global(p);

    math::quat q;
    q.x = eorientation.x;
    q.y = eorientation.y;
    q.z = eorientation.z;
    q.w = eorientation.w;
    transform.set_rotation_global(q);

    transform.set_dirty(system_id, false);
}
}

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
    auto& def = ctx.get<defaults>();

    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    auto config = edyn::init_config{};
    config.execution_mode = edyn::execution_mode::asynchronous;
    edyn::attach(registry, config);

    registry.view<box_collider_component>().each(
    [&](auto e, auto&& comp)
    {
        comp.on_phyiscs_simulation_begin();
    });

    registry.view<rigidbody_component>().each(
    [&](auto e, auto&& comp)
    {
        comp.on_phyiscs_simulation_begin();
    });
}

void physics_system::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    registry.view<box_collider_component>().each(
    [&](auto e, auto&& comp)
    {
        comp.on_phyiscs_simulation_end();
    });

    registry.view<rigidbody_component>().each(
    [&](auto e, auto&& comp)
    {
        comp.on_phyiscs_simulation_end();
    });

    edyn::detach(registry);
}

void physics_system::on_pause(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    edyn::set_paused(registry, true);
}

void physics_system::on_resume(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    edyn::set_paused(registry, false);
}

void physics_system::on_skip_next_frame(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    edyn::step_simulation(registry);
}

void physics_system::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& ev = ctx.get<events>();

    if(ev.is_playing)
    {
        auto& ec = ctx.get<ecs>();
        auto& registry = *ec.get_scene().registry;

        // update phyiscs spatial properties from transform
        registry.view<transform_component, edyn::position, edyn::orientation>().each(
        [&](auto e, auto&& transform, auto&& epos, auto&& eorientation)
        {
            to_physics(transform, epos, eorientation);
        });

        //update physics
        edyn::update(registry);


        // update transform from phyiscs interpolated spatial properties
        registry.view<transform_component, edyn::present_position, edyn::present_orientation>().each(
        [&](auto e, auto&& transform, auto&& epos, auto&& eorientation)
        {
            from_physics(transform, epos, eorientation);
        });
    }
}

} // namespace ace
