#include "physics_system.h"
#include "math/transform.hpp"
#include <engine/events.h>
#include <engine/defaults/defaults.h>

#include <engine/ecs/ecs.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/components/physics_component.h>

#include <logging/logging.h>
#include <edyn/edyn.hpp>

namespace ace
{

namespace
{
const uint8_t system_id = 1;

void to_physics(transform_component& transform, phyisics_component& rigidbody)
{
    bool transform_dirty = transform.is_dirty(system_id);
    bool rigidbody_dirty = rigidbody.is_dirty(system_id);

    if(transform_dirty || rigidbody_dirty)
    {
        rigidbody.sync_transforms(transform.get_transform_global());
    }
}

void from_physics(transform_component& transform, phyisics_component& rigidbody)
{
    auto transform_global = transform.get_transform_global();
    if(rigidbody.sync_transforms(transform_global))
    {
        transform.set_transform_global(transform_global);
    }

    transform.set_dirty(system_id, false);
    rigidbody.set_dirty(system_id, false);
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
    config.execution_mode = edyn::execution_mode::sequential;
    edyn::attach(registry, config);


    registry.view<phyisics_component>().each(
    [&](auto e, auto&& comp)
    {
        comp.on_phyiscs_simulation_begin();
    });
}

void physics_system::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    registry.view<phyisics_component>().each(
    [&](auto e, auto&& comp)
    {
        comp.on_phyiscs_simulation_end();
    });

    edyn::update(registry);

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
        registry.view<transform_component, phyisics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            to_physics(transform, rigidbody);
        });

        //update physics
        edyn::update(registry);


        // update transform from phyiscs interpolated spatial properties
        registry.view<transform_component, phyisics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            from_physics(transform, rigidbody);
        });
    }
}

} // namespace ace
