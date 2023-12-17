#include "ecs.h"
#include "components/camera_component.h"
#include "components/id_component.h"
#include "components/transform_component.h"

#include "systems/deferred_rendering.h"
#include <engine/events.h>
#include <logging/logging.h>

namespace ace
{

namespace
{
void setup_transform_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<transform_component>();
    component.set_owner(entity);
}
} // namespace

ecs::ecs()
{
    registry.on_construct<transform_component>().connect<&setup_transform_component>();
}

auto ecs::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();
    ev.on_frame_update.connect(sentinel_, this, &ecs::on_frame_update);
    ev.on_frame_render.connect(sentinel_, 900, this, &ecs::on_frame_render);

    return true;
}

auto ecs::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    close_project();

    return true;
}

void ecs::on_frame_update(rtti::context& ctx, delta_t /*dt*/)
{
    registry.view<transform_component, camera_component>().each(
        [&](auto e, auto&& transform, auto&& camera)
        {
            camera.update(transform.get_transform_global());
        });
}

void ecs::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& dr = ctx.get<deferred_rendering>();
    auto& ec = ctx.get<ecs>();

    registry.view<camera_component>().each(
        [&](auto e, auto&& camera_comp)
        {
            // entt::handle entity(registry, e);
            // auto& camera_lods = lod_data_[entity];
            lod_data_container camera_lods{};
            auto& camera = camera_comp.get_camera();
            auto& render_view = camera_comp.get_render_view();

            auto output = dr.camera_render_full(camera, render_view, ec, camera_lods, dt);
        });
}

void ecs::close_project()
{
    registry.clear();
}

auto ecs::create_entity(entt::handle parent) -> entt::handle
{
    entt::handle ent(registry, registry.create());
    ent.emplace<id_component>();
    ent.emplace<tag_component>("Entity");

    auto& transform = ent.emplace<transform_component>();
    if(parent)
    {
        transform.set_parent(parent);
    }

    return ent;
}

auto ecs::create_test_scene() -> entt::handle
{
    auto root = create_entity();
    auto e1 = create_entity();
    e1.get<transform_component>().set_parent(root);

    auto e2 = create_entity();
    e2.get<transform_component>().set_parent(e1);

    return root;
}

} // namespace ace
