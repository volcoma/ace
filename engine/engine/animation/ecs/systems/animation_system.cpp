#include "animation_system.h"
#include <engine/animation/animation.h>
#include <engine/animation/ecs/components/animation_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/events.h>
#include <engine/rendering/ecs/components/model_component.h>

#include <engine/ecs/ecs.h>
#include <engine/engine.h>
#include <engine/profiler/profiler.h>
#include <engine/threading/threader.h>
#include <logging/logging.h>

#define POOLSTL_STD_SUPPLEMENT 1
#include <poolstl/poolstl.hpp>

namespace ace
{

auto animation_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);
    auto& ev = ctx.get<events>();

    ev.on_play_begin.connect(sentinel_, 0, this, &animation_system::on_play_begin);
    ev.on_play_end.connect(sentinel_, 0, this, &animation_system::on_play_end);
    ev.on_pause.connect(sentinel_, 0, this, &animation_system::on_pause);
    ev.on_resume.connect(sentinel_, 0, this, &animation_system::on_resume);
    ev.on_skip_next_frame.connect(sentinel_, 0, this, &animation_system::on_skip_next_frame);

    return true;
}

auto animation_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void animation_system::on_create_component(entt::registry& r, const entt::entity e)
{
    auto& ctx = engine::context();
    auto& ev = ctx.get<events>();
    if(ev.is_playing)
    {
        entt::handle entity(r, e);

        auto& animation_comp = entity.get<animation_component>();

        if(animation_comp.get_autoplay())
        {
            auto& player = animation_comp.get_player();
            player.play();
        }
    }
}

void animation_system::on_destroy_component(entt::registry& r, const entt::entity e)
{
}

void animation_system::on_play_begin(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    scn.registry->view<animation_component>().each(
        [&](auto e, auto&& animation_comp)
        {
            if(animation_comp.get_autoplay())
            {
                auto& player = animation_comp.get_player();
                player.play();
            }
        });
}

void animation_system::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    scn.registry->view<animation_component>().each(
        [&](auto e, auto&& animation_comp)
        {
            auto& player = animation_comp.get_player();
            player.stop();
        });
}

void animation_system::on_pause(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    scn.registry->view<animation_component>().each(
        [&](auto e, auto&& animation_comp)
        {
            auto& player = animation_comp.get_player();
            player.pause();
        });
}

void animation_system::on_resume(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    scn.registry->view<animation_component>().each(
        [&](auto e, auto&& animation_comp)
        {
            auto& player = animation_comp.get_player();
            player.resume();
        });
}

void animation_system::on_skip_next_frame(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    delta_t step(1.0f / 60.0f);
    on_update(scn, step, true);
}

void animation_system::on_update(scene& scn, delta_t dt, bool force)
{
    APP_SCOPE_PERF("Animation System");

    auto& ctx = engine::context();
    auto& th = ctx.get<threader>();
    // Create a view for entities with transform_component and submesh_component
    auto view = scn.registry->view<model_component, animation_component>();

    // this code should be thread safe as each task works with a whole hierarchy and
    // there is no interleaving between tasks.
    std::for_each(std::execution::par,
                  view.begin(),
                  view.end(),
                  [&](entt::entity entity)
                  {
                      auto& animation_comp = view.get<animation_component>(entity);
                      auto& model_comp = view.get<model_component>(entity);

                      if(animation_comp.get_culling_mode() == animation_component::culling_mode::renderer_based)
                      {
                          return;
                      }

                      auto& player = animation_comp.get_player();

                      player.blend_to(animation_comp.get_animation());

                      player.update(
                          dt,
                          [&](/*const std::string& node_id, */ size_t node_index, const math::transform& transform)
                          {
                              auto armature = model_comp.get_armature_by_index(node_index);
                              if(armature)
                              {
                                  auto& armature_transform_comp = armature.template get<transform_component>();
                                  armature_transform_comp.set_transform_local(transform);
                              }
                          },
                          force);
                  });
}

void animation_system::on_frame_update(scene& scn, delta_t dt)
{
    on_update(scn, dt, false);
}

} // namespace ace
