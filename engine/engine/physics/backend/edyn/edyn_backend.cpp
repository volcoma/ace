#include "edyn_backend.h"
#include "rigidbody_ex.h"

#include <engine/defaults/defaults.h>
#include <engine/events.h>
#include <math/transform.hpp>

#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/physics/ecs/components/physics_component.h>

#include <edyn/edyn.hpp>
#include <logging/logging.h>

namespace ace
{

namespace
{
const uint8_t system_id = 1;

auto max3(const math::vec3& v) -> float
{
    return math::max(math::max(v.x, v.y), v.z);
}

void update_def_material(physics_component& rigidbody, edyn::rigidbody_def& def)
{
    if(rigidbody.is_sensor())
    {
        def.material.reset();
    }
    else
    {
        edyn::material emat{};
        {
            const auto& mat = rigidbody.get_material().get();
            emat.restitution = mat.restitution;
            emat.friction = mat.friction;
            emat.spin_friction = mat.spin_friction;
            emat.roll_friction = mat.roll_friction;
            emat.stiffness = mat.stiffness;
            emat.damping = mat.damping;
        }
        def.material = emat;
    }
}

void update_def_mass(physics_component& rigidbody, edyn::rigidbody_def& def)
{
    def.mass = rigidbody.get_mass();
}

void update_def_gravity(physics_component& rigidbody, edyn::rigidbody_def& def)
{
    auto owner = rigidbody.get_owner();

    auto registry = owner.registry();
    def.gravity = rigidbody.is_using_gravity() ? edyn::get_gravity(*registry) : edyn::vector3_zero;
}

void update_def_kind(physics_component& rigidbody, edyn::rigidbody_def& def)
{
    def.kind = rigidbody.is_kinematic() ? edyn::rigidbody_kind::rb_kinematic : edyn::rigidbody_kind::rb_dynamic;
}

void update_def_shape(physics_component& rigidbody, edyn::rigidbody_def& def)
{
    auto owner = rigidbody.get_owner();
    auto compound_shapes = rigidbody.get_shapes();
    if(compound_shapes.empty())
    {
        def.shape = {};
        def.inertia = edyn::matrix3x3_identity;
    }
    else
    {
        edyn::compound_shape cp;

        const auto& transform = owner.get<transform_component>();
        const auto& scale = transform.get_scale_global();

        for(const auto& s : compound_shapes)
        {
            if(std::holds_alternative<physics_box_shape>(s.shape))
            {
                auto& shape = std::get<physics_box_shape>(s.shape);
                auto extends = shape.extends * scale;

                edyn::box_shape box_shape{extends.x * 0.5f, extends.y * 0.5f, extends.z * 0.5f};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(box_shape, center, edyn::quaternion_identity);
            }
            else if(std::holds_alternative<physics_sphere_shape>(s.shape))
            {
                auto& shape = std::get<physics_sphere_shape>(s.shape);
                auto radius = shape.radius * max3(scale);

                edyn::sphere_shape sphere_shape{radius};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(sphere_shape, center, edyn::quaternion_identity);
            }
            else if(std::holds_alternative<physics_capsule_shape>(s.shape))
            {
                auto& shape = std::get<physics_capsule_shape>(s.shape);
                auto radius = shape.radius * max3(scale);
                auto half_length = shape.length * 0.5f * max3(scale);

                edyn::capsule_shape capsule_shape{radius, half_length, edyn::coordinate_axis::y};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(capsule_shape, center, edyn::quaternion_identity);
            }
            else if(std::holds_alternative<physics_cylinder_shape>(s.shape))
            {
                auto& shape = std::get<physics_cylinder_shape>(s.shape);
                auto radius = shape.radius * max3(scale);
                auto half_length = shape.length * 0.5f * max3(scale);

                edyn::cylinder_shape cylinder_shape{radius, half_length, edyn::coordinate_axis::y};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(cylinder_shape, center, edyn::quaternion_identity);
            }
        }
        cp.finish();
        def.shape = cp;
        def.inertia.reset();
    }
}

void recreate_phyisics_entity(physics_component& rigidbody)
{
    auto owner = rigidbody.get_owner();
    auto& body = owner.get_or_emplace<edyn::rigidbody>();

    if(body.internal)
    {
        body.internal.destroy();
    }

    auto registry = owner.registry();
    body.internal = entt::handle(*registry, registry->create());
    body.internal.emplace<edyn::rigidbody_owner>(owner);
}

void recreate_phyisics_body(physics_component& rigidbody, bool force = false)
{
    bool needs_recreation = force;// || rigidbody.is_property_dirty(physics_property::kind);

    if(needs_recreation)
    {
        recreate_phyisics_entity(rigidbody);
    }

    auto owner = rigidbody.get_owner();
    auto& body = owner.get<edyn::rigidbody>();
    auto internal_entity = body.internal;
    auto entity = internal_entity.entity();
    auto& registry = *internal_entity.registry();

    update_def_mass(rigidbody, body.def);
    update_def_kind(rigidbody, body.def);
    update_def_shape(rigidbody, body.def);
    update_def_material(rigidbody, body.def);
    update_def_gravity(rigidbody, body.def);

    if(needs_recreation)
    {
        edyn::make_rigidbody(entity, registry, body.def);
    }
    else
    {

        if(rigidbody.is_property_dirty(physics_property::mass))
        {
            edyn::update_rigidbody_mass(entity, registry, body.def);
        }
        if(rigidbody.is_property_dirty(physics_property::kind))
        {
            edyn::update_rigidbody_kind(entity, registry, body.def);
        }
        if(rigidbody.is_property_dirty(physics_property::gravity))
        {
            edyn::update_rigidbody_gravity(entity, registry, body.def);
        }
        if(rigidbody.is_property_dirty(physics_property::material))
        {
            edyn::update_rigidbody_material(entity, registry, body.def);
        }
        if(rigidbody.is_property_dirty(physics_property::shape))
        {
            edyn::update_rigidbody_shape(entity, registry, body.def);
        }
    }

    rigidbody.set_dirty(system_id, false);
}

void destroy_phyisics_body(physics_component& rigidbody)
{
    auto owner = rigidbody.get_owner();
    auto body = owner.try_get<edyn::rigidbody>();

    if(body->internal)
    {
        body->internal.destroy();
    }

    owner.remove<edyn::rigidbody>();
}

void sync_transforms(physics_component& rigidbody, const math::transform& transform)
{
    auto owner = rigidbody.get_owner();
    auto& body = owner.get<edyn::rigidbody>();

    if(!body.internal)
    {
        return;
    }

    auto pe = body.internal.entity();
    auto& preg = *body.internal.registry();

    if(rigidbody.is_kinematic())
    {
        edyn::position epos{};
        const auto& p = transform.get_position();
        epos.x = p.x;
        epos.y = p.y;
        epos.z = p.z;
        edyn::update_kinematic_position(preg, pe, epos, 1.0f);
        preg.patch<edyn::linvel>(pe);

        edyn::orientation eorientation{};
        const auto& q = transform.get_rotation();
        eorientation.x = q.x;
        eorientation.y = q.y;
        eorientation.z = q.z;
        eorientation.w = q.w;
        edyn::update_kinematic_orientation(preg, pe, eorientation, 1.0f);
        preg.patch<edyn::angvel>(pe);
    }
    else
    {
        auto [epos, eorientation] = body.internal.get<edyn::position, edyn::orientation>();

        const auto& p = transform.get_position();
        epos.x = p.x;
        epos.y = p.y;
        epos.z = p.z;

        const auto& q = transform.get_rotation();
        eorientation.x = q.x;
        eorientation.y = q.y;
        eorientation.z = q.z;
        eorientation.w = q.w;
    }

    body.internal.patch<edyn::position>();
    body.internal.patch<edyn::orientation>();

    edyn::wake_up_entity(preg, pe);
}

auto sync_transforms(physics_component& rigidbody, math::transform& transform) -> bool
{
    auto owner = rigidbody.get_owner();
    auto& body = owner.get<edyn::rigidbody>();

    if(!body.internal)
    {
        return false;
    }

    auto [epos, eorientation] = body.internal.try_get<edyn::present_position, edyn::present_orientation>();

    if(epos)
    {
        math::vec3 p;
        p.x = epos->x;
        p.y = epos->y;
        p.z = epos->z;
        transform.set_position(p);
    }

    if(eorientation)
    {
        math::quat q;
        q.x = eorientation->x;
        q.y = eorientation->y;
        q.z = eorientation->z;
        q.w = eorientation->w;
        transform.set_rotation(q);
    }

    return epos || eorientation;
}

void to_physics(transform_component& transform, physics_component& rigidbody)
{
    bool transform_dirty = transform.is_dirty(system_id);
    bool rigidbody_dirty = rigidbody.is_dirty(system_id);

    if(rigidbody_dirty)
    {
        recreate_phyisics_body(rigidbody);
    }

    if(transform_dirty || rigidbody_dirty)
    {
        sync_transforms(rigidbody, transform.get_transform_global());
    }
}

void from_physics(transform_component& transform, physics_component& rigidbody)
{
    auto transform_global = transform.get_transform_global();
    if(sync_transforms(rigidbody, transform_global))
    {
        transform.set_transform_global(transform_global);
    }

    transform.set_dirty(system_id, false);
    rigidbody.set_dirty(system_id, false);
}

void sensor_contact_started(entt::registry& registry, entt::entity entity)
{
    auto& manifold = registry.get<edyn::contact_manifold>(entity);
    for(auto body : manifold.body)
    {
        auto pbody = registry.try_get<edyn::rigidbody_owner>(body);
        if(pbody)
        {
            auto pe = pbody->owner;
        }
    }
}

void sensor_contact_ended(entt::registry& registry, entt::entity entity)
{
    auto& manifold = registry.get<edyn::contact_manifold>(entity);
    for(auto body : manifold.body)
    {
        auto pbody = registry.try_get<edyn::rigidbody_owner>(body);
        if(pbody)
        {
            auto pe = pbody->owner;
        }
    }
}

} // namespace

void edyn_backend::on_create_component(entt::registry& r, const entt::entity e)
{
}

void edyn_backend::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& rigidbody = entity.get<physics_component>();
    destroy_phyisics_body(rigidbody);
}

void edyn_backend::on_apply_impulse(physics_component& comp, const math::vec3& impulse)
{
    auto owner = comp.get_owner();
    auto& registry = *owner.registry();

    auto& ebody = owner.get<edyn::rigidbody>();
    edyn::rigidbody_apply_impulse(registry, ebody.internal, {impulse.x, impulse.y, impulse.z}, edyn::vector3_zero);

    edyn::wake_up_entity(registry, ebody.internal);
}

void edyn_backend::on_apply_torque_impulse(physics_component& comp, const math::vec3& impulse)
{
    auto owner = comp.get_owner();
    auto& registry = *owner.registry();

    auto& ebody = owner.get<edyn::rigidbody>();

    edyn::rigidbody_apply_torque_impulse(registry, ebody.internal, {impulse.x, impulse.y, impulse.z});

    edyn::wake_up_entity(registry, ebody.internal);
}

void edyn_backend::on_clear_kinematic_velocities(physics_component& comp)
{
    if(comp.is_kinematic())
    {
        auto owner = comp.get_owner();
        auto& registry = *owner.registry();

        auto& ebody = owner.get<edyn::rigidbody>();

        auto [lvel, avel] = ebody.internal.try_get<edyn::linvel, edyn::angvel>();

        if(lvel)
        {
            *lvel = edyn::vector3_zero;
            ebody.internal.patch<edyn::linvel>();
        }

        if(avel)
        {
            *avel = edyn::vector3_zero;
            ebody.internal.patch<edyn::linvel>();
        }

        edyn::wake_up_entity(registry, ebody.internal);
    }
}

void edyn_backend::on_play_begin(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    auto config = edyn::init_config{};
    config.execution_mode = edyn::execution_mode::asynchronous;
    edyn::attach(registry, config);

    edyn::on_contact_started(registry).connect<&sensor_contact_started>(registry);
    edyn::on_contact_ended(registry).connect<&sensor_contact_ended>(registry);

    registry.view<physics_component>().each(
        [&](auto e, auto&& comp)
        {
            recreate_phyisics_body(comp, true);
        });
}

void edyn_backend::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    registry.view<physics_component>().each(
        [&](auto e, auto&& comp)
        {
            destroy_phyisics_body(comp);
        });

    edyn::update(registry);

    edyn::on_contact_started(registry).disconnect<&sensor_contact_started>(registry);
    edyn::on_contact_ended(registry).disconnect<&sensor_contact_ended>(registry);

    edyn::detach(registry);
}

void edyn_backend::on_pause(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    edyn::set_paused(registry, true);
}

void edyn_backend::on_resume(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    edyn::set_paused(registry, false);
}

void edyn_backend::on_skip_next_frame(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    edyn::step_simulation(registry);
}

void edyn_backend::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    if(os::key::is_pressed(os::key::code::space))
    {
        // update phyiscs spatial properties from transform
        registry.view<transform_component, physics_component>().each(
            [&](auto e, auto&& transform, auto&& rigidbody)
            {
                rigidbody.apply_impulse({0.0f, 2.0f, 0.0f});
            });
    }

    if(os::key::is_pressed(os::key::code::enter))
    {
        // update phyiscs spatial properties from transform
        registry.view<transform_component, physics_component>().each(
            [&](auto e, auto&& transform, auto&& rigidbody)
            {
                rigidbody.apply_impulse({1.0f, 1.0f, 0.0f});
            });
    }

    // update phyiscs spatial properties from transform
    registry.view<transform_component, physics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            to_physics(transform, rigidbody);
        });

    // update physics
    edyn::update(registry);

    // update transform from phyiscs interpolated spatial properties
    registry.view<transform_component, physics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            from_physics(transform, rigidbody);
        });
}

} // namespace ace
