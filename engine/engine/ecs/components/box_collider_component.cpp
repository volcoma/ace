#include "box_collider_component.h"
#include "edyn/util/aabb_util.hpp"
#include "logging/logging.h"
#include <edyn/edyn.hpp>

#include <cstdint>

namespace ace
{

namespace
{
auto is_simulation_active(const entt::registry& registry) -> bool
{
    return registry.ctx().contains<edyn::settings>();
}

} // namespace

void box_collider_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<box_collider_component>();
    component.set_owner(entity);
    component.on_phyiscs_simulation_begin();
}

void box_collider_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& component = entity.get<box_collider_component>();
    component.on_phyiscs_simulation_end();

}

void box_collider_component::on_change_extends()
{
    auto entity = get_owner();
    auto& registry = *entity.registry();
    if(!is_simulation_active(registry))
    {
        return;
    }

    auto extends = get_extends();
    auto shape = edyn::box_shape{extends.x * 0.5f, extends.y * 0.5f, extends.z * 0.5f};

    // Ensure shape is valid for this type of rigid body.
//    if (def.kind != edyn::rigidbody_kind::rb_static) {
        EDYN_ASSERT((!edyn::tuple_has_type<edyn::box_shape, edyn::static_shapes_tuple_t>::value));
//    }


        APPLOG_INFO("BEFORE");

        for(auto [id, storage] : registry.storage())
        {
            auto name = storage.type().name();
            if(storage.contains(entity))
            {
                APPLOG_INFO("storage {}", std::string(name));
            }
        }


    registry.emplace_or_replace<edyn::box_shape>(entity, shape);
//    registry.patch<edyn::box_shape>(entity);

    registry.emplace_or_replace<edyn::shape_index>(entity, edyn::get_shape_index<edyn::box_shape>());
//    registry.patch<edyn::shape_index>(entity);

    auto aabb = edyn::shape_aabb(shape, edyn::vector3_zero, edyn::quaternion_identity);
    registry.emplace_or_replace<edyn::AABB>(entity, aabb);
//    registry.patch<edyn::AABB>(entity);

//    // Assign tag for rolling shapes.
//    if (def.kind == edyn::rigidbody_kind::rb_dynamic) {
//        if constexpr(edyn::tuple_has_type<ShapeType, edyn::rolling_shapes_tuple_t>::value) {
//            registry.emplace<edyn::rolling_tag>(entity);

//            auto roll_dir = shape_rolling_direction(shape);

//            if (roll_dir != edyn::vector3_zero) {
//                registry.emplace<edyn::roll_direction>(entity, roll_dir);
//            }
//        }
//    }
    uint64_t collision_group {edyn::collision_filter::all_groups};
    uint64_t collision_mask {edyn::collision_filter::all_groups};

    if (collision_group != edyn::collision_filter::all_groups ||
        collision_mask != edyn::collision_filter::all_groups)
    {
        auto &filter = registry.emplace_or_replace<edyn::collision_filter>(entity);
        filter.group = collision_group;
        filter.mask = collision_mask;

//        registry.patch<edyn::collision_filter>(entity);
    }


    APPLOG_INFO("AFTER");

    for(auto [id, storage] : registry.storage())
    {
        auto name = storage.type().name();
        if(storage.contains(entity))
        {
            APPLOG_INFO("storage {}", std::string(name));
        }
    }


}

void box_collider_component::set_extends(const math::vec3& extends)
{
    if(math::all(math::epsilonEqual(extends_, extends, math::epsilon<float>())))
    {
        return;
    }

    extends_ = extends;

    on_change_extends();
}

auto box_collider_component::get_extends() const noexcept -> const math::vec3&
{
    return extends_;
}

void box_collider_component::on_phyiscs_simulation_begin()
{
    on_change_extends();
}

void box_collider_component::on_phyiscs_simulation_end()
{
    auto entity = get_owner();
    entity.remove<edyn::box_shape>();
    entity.remove<edyn::shape_index>();
    entity.remove<edyn::AABB>();
    entity.remove<edyn::collision_filter>();

}


} // namespace ace
