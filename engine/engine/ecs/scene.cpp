#include "scene.h"
#include "components/id_component.h"
#include "components/physics_component.h"
#include "components/transform_component.h"

#include <engine/events.h>
#include <engine/meta/ecs/entity.hpp>
#include <logging/logging.h>

#include <chrono>

namespace ace
{

namespace
{

auto clone_entity_impl(entt::registry& r, entt::handle entity) -> entt::handle
{
    entt::handle object(r, r.create());

    bool is_rigidbody{};
    for(auto [id, storage] : r.storage())
    {
        auto name = storage.type().name();

        if(name.find("edyn::") != std::string_view::npos)
        {
            continue;
        }

        if(storage.contains(entity) && !storage.contains(object))
        {
            storage.push(object, storage.value(entity));
        }
    }

    return object;
}

} // namespace

scene::scene()
{
    registry = std::make_unique<entt::registry>();
    registry->on_construct<transform_component>().connect<&transform_component::on_create_component>();
    registry->on_destroy<transform_component>().connect<&transform_component::on_destroy_component>();

    registry->on_construct<physics_component>().connect<&physics_component::on_create_component>();
    registry->on_destroy<physics_component>().connect<&physics_component::on_destroy_component>();

}

scene::~scene()
{
    unload();
}

void scene::unload()
{
    registry->clear();
}

auto scene::load_from(const asset_handle<scene_prefab>& pfb) -> bool
{
    if(load_from_prefab(pfb, *this))
    {
        source = pfb;
        return true;
    }

    return false;
}

auto scene::instantiate(const asset_handle<prefab>& pfb) -> entt::handle
{
    return load_from_prefab(pfb, *registry);
}

auto scene::create_entity(const std::string& tag, entt::handle parent) -> entt::handle
{
    entt::handle ent(*registry, registry->create());
    ent.emplace<tag_component>(!tag.empty() ? tag : "Entity");

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

auto scene::clone_entity(entt::handle clone_from, bool keep_parent) -> entt::handle
{
    auto clone_to = clone_entity_impl(*registry, clone_from);

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

void scene::clone_scene(const scene& src_scene, scene& dst_scene)
{
    clone_scene_from_stream(src_scene, dst_scene);
}

auto scene::create_entity(entt::entity e) -> entt::handle
{
    entt::handle handle(*registry, e);
    return handle;
}

auto scene::create_entity(entt::entity e) const -> entt::const_handle
{
    entt::const_handle handle(*registry, e);
    return handle;
}

} // namespace ace
