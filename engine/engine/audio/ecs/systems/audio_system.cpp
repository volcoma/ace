#include "audio_system.h"
#include <engine/events.h>

#include <engine/audio/ecs/components/audio_listener_component.h>
#include <engine/audio/ecs/components/audio_source_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>

#include <audiopp/logger.h>
#include <logging/logging.h>

namespace ace
{

namespace
{

void on_create_component(entt::registry& r, const entt::entity e)
{
    auto& comp = r.get<audio_source_component>(e);
    comp.on_play_begin();
}
void on_destroy_component(entt::registry& r, const entt::entity e)
{
    auto& comp = r.get<audio_source_component>(e);
    comp.on_play_end();
}

} // namespace

auto audio_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();
    ev.on_frame_update.connect(sentinel_, this, &audio_system::on_frame_update);

    ev.on_play_begin.connect(sentinel_, -100, this, &audio_system::on_play_begin);
    ev.on_play_end.connect(sentinel_, 100, this, &audio_system::on_play_end);
    ev.on_pause.connect(sentinel_, -100, this, &audio_system::on_pause);
    ev.on_resume.connect(sentinel_, 100, this, &audio_system::on_resume);
    ev.on_skip_next_frame.connect(sentinel_, -100, this, &audio_system::on_skip_next_frame);

    audio::set_info_logger(
        [](const std::string& s)
        {
            APPLOG_INFO(s);
        });
    audio::set_error_logger(
        [](const std::string& s)
        {
            APPLOG_ERROR(s);
        });

    audio::set_trace_logger(
        [](const std::string& s)
        {
            APPLOG_TRACE(s);
        });

    device_ = std::make_unique<audio::device>();

    return true;
}

auto audio_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void audio_system::on_play_begin(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    registry.on_construct<audio_source_component>().connect<&on_create_component>();
    registry.on_destroy<audio_source_component>().connect<&on_destroy_component>();

    registry.view<audio_source_component>().each(
        [&](auto e, auto&& comp)
        {
            comp.on_play_begin();
        });
}

void audio_system::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    registry.view<audio_source_component>().each(
        [&](auto e, auto&& comp)
        {
            comp.on_play_end();
        });

    registry.on_construct<audio_source_component>().disconnect<&on_create_component>();
    registry.on_destroy<audio_source_component>().disconnect<&on_destroy_component>();
}

void audio_system::on_pause(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    registry.view<audio_source_component>().each(
        [&](auto e, auto&& comp)
        {
            comp.pause();
        });
}

void audio_system::on_resume(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    registry.view<audio_source_component>().each(
        [&](auto e, auto&& comp)
        {
            comp.resume();
        });
}

void audio_system::on_skip_next_frame(rtti::context& ctx)
{
}

void audio_system::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& ev = ctx.get<events>();

    if(ev.is_playing && !ev.is_paused)
    {
    }

    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    // update auidio spatial properties from transform
    registry.view<transform_component, audio_listener_component>().each(
        [&](auto e, auto&& transform, auto&& comp)
        {
            comp.update(transform.get_transform_global(), dt);
        });

    registry.view<transform_component, audio_source_component>().each(
        [&](auto e, auto&& transform, auto&& comp)
        {
            comp.update(transform.get_transform_global(), dt);
        });
}

} // namespace ace
