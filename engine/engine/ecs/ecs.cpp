#include "ecs.h"
#include "components/camera_component.h"
#include "components/id_component.h"
#include "components/transform_component.h"

#include "systems/deferred_rendering.h"
#include <engine/meta/ecs/entity.hpp>
#include <engine/events.h>
#include <logging/logging.h>

namespace ace
{

namespace
{
auto clone_entity_impl(entt::registry& r, entt::handle entity) -> entt::handle
{
    entt::handle object(r, r.create());

    for(auto [id, storage] : r.storage())
    {
        if(storage.contains(entity) && !storage.contains(object))
        {
            storage.push(object, storage.value(entity));
        }
    }

    return object;
}

} // namespace

ecs::ecs()
{
    registry.on_construct<transform_component>().connect<&transform_component::on_create_component>();
    registry.on_destroy<transform_component>().connect<&transform_component::on_destroy_component>();
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

auto ecs::instantiate(const asset_handle<prefab>& pfb) -> entt::handle
{
    return load_from_prefab(pfb, registry);
}

auto ecs::create_entity(entt::handle parent) -> entt::handle
{
    entt::handle ent(registry, registry.create());
    ent.emplace<tag_component>("Entity");

    auto& transform = ent.emplace<transform_component>();
    if(parent)
    {
        set_parent_params params;
        params.global_transform_stays = false;
        params.local_transform_stays = true;
        transform.set_parent(parent, params);
    }

    return ent;
}

auto ecs::clone_entity(entt::handle clone_from, bool keep_parent) -> entt::handle
{
    auto clone_to = clone_entity_impl(registry, clone_from);

    // get cloned to transform
    auto& clone_to_component = clone_to.get<transform_component>();

    // clear parent and children which were copied.
    clone_to_component._clear_relationships();

    // get cloned from transform
    auto& clone_from_component = clone_from.get<transform_component>();

    // clone children as well
    const auto& children = clone_from_component.get_children();
    for(const auto& child : children)
    {
        auto cloned_child = clone_entity(child, false);
        auto& comp = cloned_child.get<transform_component>();
        comp.set_parent(clone_to);
    }

    if(keep_parent)
    {
        // set parent from original
        auto parent = clone_from_component.get_parent();
        if(parent)
        {
            clone_to_component.set_parent(parent);
        }
    }


    return clone_to;
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
