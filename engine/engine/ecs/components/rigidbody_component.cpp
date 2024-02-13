#include "rigidbody_component.h"
#include "edyn/comp/gravity.hpp"
#include "edyn/comp/present_orientation.hpp"
#include <edyn/edyn.hpp>

#include <cstdint>

namespace edyn
{

void remove_rigidbody(entt::entity entity, entt::registry& registry)
{
    //    if(auto node = registry.try_get<graph_node>(entity))
    //    {
    //        registry.ctx().get<entity_graph>().remove_node(node->node_index);
    //    }

    registry.remove<dynamic_tag,
                    procedural_tag,
                    kinematic_tag,
                    static_tag,
                    position,
                    orientation,
                    mass,
                    mass_inv,
                    inertia,
                    inertia_inv,
                    inertia_world_inv,
                    linvel,
                    angvel,
                    gravity,

                    material,

                    present_position,
                    present_orientation,

                    dynamic_tag,
                    procedural_tag,
                    kinematic_tag,
                    static_tag,
                    sleeping_disabled_tag,
                    networked_tag,
                    graph_node,
                    graph_edge,
                    multi_island_resident,
                    island_resident,
                    rigidbody_tag>(entity);
}

void update_rigidbody(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    if(!registry.all_of<position>(entity))
    {
        registry.emplace_or_replace<position>(entity, def.position);
        registry.patch<position>(entity);
    }

    if(!registry.all_of<orientation>(entity))
    {
        registry.emplace_or_replace<orientation>(entity, def.orientation);
        registry.patch<orientation>(entity);
    }


    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        EDYN_ASSERT(def.mass > EDYN_EPSILON && def.mass < large_scalar);
        registry.emplace_or_replace<mass>(entity, def.mass);
        registry.patch<mass>(entity);

        registry.emplace_or_replace<mass_inv>(entity, scalar(1) / def.mass);
        registry.patch<mass_inv>(entity);

        matrix3x3 inertia_mat = matrix3x3_identity;

        //        if (def.inertia) {
        //            inertia = *def.inertia;
        //        } else if(def.shape) {
        //            inertia = edyn::edmoment_of_inertia(*def.shape, def.mass);

        //            if (def.center_of_mass) {
        //                // Use parallel-axis theorem to calculate moment of inertia along
        //                // axes away from the origin.
        //                inertia = shift_moment_of_inertia(inertia, def.mass, *def.center_of_mass);
        //            }
        //        }

        auto I_inv = inverse_matrix_symmetric(inertia_mat);
        registry.emplace_or_replace<inertia>(entity, inertia_mat);
        registry.patch<inertia>(entity);

        registry.emplace_or_replace<inertia_inv>(entity, I_inv);
        registry.patch<inertia_inv>(entity);


        auto basis = to_matrix3x3(def.orientation);
        auto I_inv_world = basis * I_inv * transpose(basis);
        registry.emplace_or_replace<inertia_world_inv>(entity, I_inv_world);
        registry.patch<inertia_world_inv>(entity);

    }
    else
    {
        registry.emplace_or_replace<mass>(entity, EDYN_SCALAR_MAX);
        registry.patch<mass>(entity);

        registry.emplace_or_replace<mass_inv>(entity, scalar(0));
        registry.patch<mass_inv>(entity);

        registry.emplace_or_replace<inertia>(entity, matrix3x3_zero);
        registry.patch<inertia>(entity);

        registry.emplace_or_replace<inertia_inv>(entity, matrix3x3_zero);
        registry.patch<inertia_inv>(entity);

        registry.emplace_or_replace<inertia_world_inv>(entity, matrix3x3_zero);
        registry.patch<inertia_world_inv>(entity);

    }

    if(def.kind == rigidbody_kind::rb_static)
    {
        registry.emplace_or_replace<linvel>(entity, vector3_zero);
        registry.patch<linvel>(entity);
        registry.emplace_or_replace<angvel>(entity, vector3_zero);
        registry.patch<angvel>(entity);
    }
    else
    {
        registry.emplace_or_replace<linvel>(entity, def.linvel);
        registry.patch<linvel>(entity);
        registry.emplace_or_replace<angvel>(entity, def.angvel);
        registry.patch<angvel>(entity);
    }

    if(def.center_of_mass)
    {
        apply_center_of_mass(registry, entity, *def.center_of_mass);
    }

    auto g = def.gravity ? *def.gravity : get_gravity(registry);

    if(g != vector3_zero && def.kind == rigidbody_kind::rb_dynamic)
    {
        registry.emplace_or_replace<gravity>(entity, g);
        registry.patch<gravity>(entity);

    }
    else
    {
        registry.remove<gravity>(entity);
    }

    if(def.material)
    {
        registry.emplace_or_replace<material>(entity, *def.material);
        registry.patch<material>(entity);

    }

    if(def.presentation && def.kind == rigidbody_kind::rb_dynamic)
    {
        if(!registry.all_of<present_position>(entity))
        {
            registry.emplace_or_replace<present_position>(entity, registry.get<position>(entity));
            registry.patch<present_position>(entity);
        }

        if(!registry.all_of<present_orientation>(entity))
        {
            registry.emplace_or_replace<present_orientation>(entity, registry.get<orientation>(entity));
            registry.patch<present_orientation>(entity);
        }

    }
    else
    {
        registry.remove<present_position>(entity);
        registry.remove<present_orientation>(entity);
    }

    switch(def.kind)
    {
        case rigidbody_kind::rb_dynamic:
            registry.remove<kinematic_tag, static_tag>(entity);
            registry.emplace_or_replace<dynamic_tag>(entity);
            registry.patch<dynamic_tag>(entity);

            registry.emplace_or_replace<procedural_tag>(entity);
            registry.patch<procedural_tag>(entity);

            break;
        case rigidbody_kind::rb_kinematic:
            registry.remove<dynamic_tag, procedural_tag, static_tag>(entity);
            registry.emplace_or_replace<kinematic_tag>(entity);
            registry.patch<kinematic_tag>(entity);

            break;
        case rigidbody_kind::rb_static:
            registry.remove<dynamic_tag, procedural_tag, kinematic_tag>(entity);
            registry.emplace_or_replace<static_tag>(entity);
            registry.patch<static_tag>(entity);

            break;
    }

    if(def.sleeping_disabled)
    {
        registry.emplace_or_replace<sleeping_disabled_tag>(entity);
        registry.patch<sleeping_disabled_tag>(entity);

    }

    if(def.networked)
    {
        registry.emplace_or_replace<networked_tag>(entity);
        registry.patch<networked_tag>(entity);

    }

    if(auto node = registry.try_get<graph_node>(entity))
    {
        registry.ctx().get<entity_graph>().remove_node(node->node_index);
    }
    // Insert rigid body as a node in the entity graph.
    auto non_connecting = def.kind != rigidbody_kind::rb_dynamic;
    auto node_index = registry.ctx().get<entity_graph>().insert_node(entity, non_connecting);
    registry.emplace_or_replace<graph_node>(entity, node_index);
    registry.patch<graph_node>(entity);

    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        registry.remove<multi_island_resident>(entity);
        registry.emplace_or_replace<island_resident>(entity);
        registry.patch<island_resident>(entity);

    }
    else
    {
        registry.remove<island_resident>(entity);
        registry.emplace_or_replace<multi_island_resident>(entity);
        registry.patch<multi_island_resident>(entity);

    }

    // Always do this last to signal the completion of the construction of this
    // rigid body.
    registry.emplace_or_replace<rigidbody_tag>(entity);
    registry.patch<rigidbody_tag>(entity);

}
} // namespace edyn

namespace ace
{

namespace
{
auto is_simulation_active(const entt::registry& registry) -> bool
{
    return registry.ctx().contains<edyn::settings>();
}

} // namespace

void rigidbody_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<rigidbody_component>();
    component.set_owner(entity);
    component.on_phyiscs_simulation_begin();
}

void rigidbody_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& component = entity.get<rigidbody_component>();
    component.on_phyiscs_simulation_end();
}

void rigidbody_component::set_is_kinematic(bool kinematic)
{
    if(is_kinematic_ == kinematic)
    {
        return;
    }

    is_kinematic_ = kinematic;

    on_change_kind();
}
auto rigidbody_component::is_kinematic() const noexcept -> bool
{
    return is_kinematic_;
}

void rigidbody_component::on_change_kind()
{
    auto entity = get_owner();
    auto& registry = *entity.registry();
    if(!is_simulation_active(registry))
    {
        return;
    }

    dirty_.set();

    edyn::rigidbody_def def;
    def.mass = mass_;

    if(is_kinematic())
    {
        def.kind = edyn::rigidbody_kind::rb_kinematic;
    }

    if(!is_using_gravity())
    {
        def.gravity = edyn::vector3{0, 0, 0};
    }

    edyn::update_rigidbody(entity.entity(), registry, def);
}

void rigidbody_component::set_is_using_gravity(bool use_gravity)
{
    if(is_using_gravity_ == use_gravity)
    {
        return;
    }

    is_using_gravity_ = use_gravity;

    on_change_gravity();
}

auto rigidbody_component::is_using_gravity() const noexcept -> bool
{
    return is_using_gravity_;
}

void rigidbody_component::on_change_gravity()
{
    auto entity = get_owner();
    auto& registry = *entity.registry();
    if(!is_simulation_active(registry))
    {
        return;
    }

    dirty_.set();

    if(is_using_gravity())
    {
        auto gravity = edyn::get_gravity(registry);

        if(!is_kinematic())
        {
            registry.emplace_or_replace<edyn::gravity>(entity, gravity);
            registry.patch<edyn::gravity>(entity);
        }
    }
    else
    {
        registry.emplace_or_replace<edyn::linvel>(entity, edyn::vector3_zero);
        registry.patch<edyn::linvel>(entity);
        registry.remove<edyn::gravity>(entity);
    }


}

void rigidbody_component::on_phyiscs_simulation_begin()
{
    auto entity = get_owner();
    auto& registry = *entity.registry();
    if(!is_simulation_active(registry))
    {
        return;
    }

    dirty_.set();

    edyn::rigidbody_def def;
    def.mass = mass_;

    if(is_kinematic())
    {
        def.kind = edyn::rigidbody_kind::rb_kinematic;
    }

    if(!is_using_gravity())
    {
        def.gravity = edyn::vector3{0, 0, 0};
    }

    edyn::make_rigidbody(entity.entity(), registry, def);
}

void rigidbody_component::on_phyiscs_simulation_end()
{
    auto entity = get_owner();
    auto& registry = *entity.registry();
    edyn::remove_rigidbody(entity.entity(), registry);
}

auto rigidbody_component::is_dirty(uint8_t id) const noexcept -> bool
{
    return dirty_[id];
}

void rigidbody_component::set_dirty(uint8_t id, bool dirty) noexcept
{
    dirty_.set(id, dirty);
}

} // namespace ace
