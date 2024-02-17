#include "rigidbody_ex.h"

namespace edyn
{

void rigidbody_shared::on_create_component(entt::registry& r, const entt::entity e)
{
}

void rigidbody_shared::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& component = entity.get<rigidbody_shared>();

    if(component.entity)
    {
        component.entity.destroy();
    }
}

auto try_get_rigidbody(entt::handle owner) -> rigidbody_shared*
{
    return owner.try_get<rigidbody_shared>();
}

auto try_get_rigidbody(entt::const_handle owner) -> const rigidbody_shared*
{
    return owner.try_get<rigidbody_shared>();
}

void recreate_ref_rigidbody(rigidbody_shared& body)
{
    if(body.entity)
    {
        auto& registry = *body.entity.registry();
        body.entity.destroy();

        body.entity = entt::handle(registry, registry.create());
    }
}

auto add_ref_rigidbody(entt::handle owner) -> rigidbody_shared&
{
    auto& body = owner.get_or_emplace<rigidbody_shared>();
    if(!body.entity)
    {
        auto& registry = *owner.registry();
        body.entity = entt::handle(registry, registry.create());
    }
    body.add_ref();
    return body;
}

void dec_ref_rigidbody(entt::handle owner)
{
    auto body = owner.try_get<rigidbody_shared>();

    if(body && body->dec_ref())
    {
        owner.remove<rigidbody_shared>();
    }
}

void update_rigidbody_mass(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        EDYN_ASSERT(def.mass > EDYN_EPSILON && def.mass < large_scalar);
        registry.emplace<mass>(entity, def.mass);
        registry.emplace<mass_inv>(entity, scalar(1) / def.mass);

        wake_up_entity(registry, entity);
    }
    else
    {
        registry.emplace<mass>(entity, EDYN_SCALAR_MAX);
        registry.emplace<mass_inv>(entity, scalar(0));
    }
}

void remove_rigidbody_shape(entt::entity entity, entt::registry& registry)
{
    //todo remove all shapes
    registry.remove<edyn::box_shape>(entity);
    registry.remove<edyn::shape_index>(entity);
    registry.remove<edyn::AABB>(entity);
    registry.remove<edyn::collision_filter>(entity);
}

void update_rigidbody_shape(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    if(def.shape)
    {
        if(def.kind == rigidbody_kind::rb_dynamic)
        {
            matrix3x3 inertia = moment_of_inertia(*def.shape, def.mass);

            if(def.center_of_mass)
            {
                // Use parallel-axis theorem to calculate moment of inertia along
                // axes away from the origin.
                inertia = shift_moment_of_inertia(inertia, def.mass, *def.center_of_mass);
            }

            auto I_inv = inverse_matrix_symmetric(inertia);
            registry.emplace_or_replace<edyn::inertia>(entity, inertia);
            registry.emplace_or_replace<inertia_inv>(entity, I_inv);

            auto basis = to_matrix3x3(def.orientation);
            auto I_inv_world = basis * I_inv * transpose(basis);
            registry.emplace_or_replace<inertia_world_inv>(entity, I_inv_world);
        }

        std::visit(
            [&](auto&& shape)
            {
                using ShapeType = std::decay_t<decltype(shape)>;

                // Ensure shape is valid for this type of rigid body.
                if(def.kind != rigidbody_kind::rb_static)
                {
                    EDYN_ASSERT((!tuple_has_type<ShapeType, static_shapes_tuple_t>::value));
                }

                registry.emplace_or_replace<ShapeType>(entity, shape);
                registry.emplace_or_replace<shape_index>(entity, get_shape_index<ShapeType>());
                auto aabb = shape_aabb(shape, def.position, def.orientation);
                registry.emplace_or_replace<AABB>(entity, aabb);

                // Assign tag for rolling shapes.
                if(def.kind == rigidbody_kind::rb_dynamic)
                {
                    if constexpr(tuple_has_type<ShapeType, rolling_shapes_tuple_t>::value)
                    {
                        registry.emplace_or_replace<rolling_tag>(entity);

                        auto roll_dir = shape_rolling_direction(shape);

                        if(roll_dir != vector3_zero)
                        {
                            registry.emplace_or_replace<roll_direction>(entity, roll_dir);
                        }
                    }
                    else
                    {
                        registry.remove<rolling_tag>(entity);
                        registry.remove<roll_direction>(entity);
                    }
                }
            },
            *def.shape);

        if(def.collision_group != collision_filter::all_groups || def.collision_mask != collision_filter::all_groups)
        {
            auto& filter = registry.emplace_or_replace<collision_filter>(entity);
            filter.group = def.collision_group;
            filter.mask = def.collision_mask;
        }

        wake_up_entity(registry, entity);
    }
    else
    {
        remove_rigidbody_shape(entity, registry);
    }
}

void update_rigidbody_gravity(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    auto g = def.gravity ? *def.gravity : get_gravity(registry);

    if(g != vector3_zero && def.kind == rigidbody_kind::rb_dynamic)
    {
        registry.emplace_or_replace<gravity>(entity, g);
    }
    else
    {
        registry.emplace_or_replace<edyn::linvel>(entity, edyn::vector3_zero);
        registry.remove<gravity>(entity);
    }

    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        edyn::wake_up_entity(registry, entity);
    }
}

} // namespace edyn
