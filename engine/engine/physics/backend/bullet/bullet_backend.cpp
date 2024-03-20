#include "bullet_backend.h"

#include <engine/defaults/defaults.h>
#include <engine/events.h>
#include <math/transform.hpp>

#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>

#include <btBulletDynamicsCommon.h>

#include <logging/logging.h>

namespace bullet
{
struct rigidbody
{
    std::shared_ptr<btRigidBody> internal{};
    std::shared_ptr<btCollisionShape> internal_shape{};
};

struct world
{
    std::shared_ptr<btBroadphaseInterface> broadphase;
    std::shared_ptr<btCollisionDispatcher> dispatcher;
    std::shared_ptr<btConstraintSolver> solver;
    std::shared_ptr<btDefaultCollisionConfiguration> collision_config;
    std::shared_ptr<btDiscreteDynamicsWorld> dynamics_world;
};

auto create_dynamics_world() -> bullet::world
{
    bullet::world world{};
    /// collision configuration contains default setup for memory, collision setup
    world.collision_config = std::make_shared<btDefaultCollisionConfiguration>();
    // m_collisionConfiguration->setConvexConvexMultipointIterations();

    /// use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see
    /// Extras/BulletMultiThreaded)
    world.dispatcher = std::make_shared<btCollisionDispatcher>(world.collision_config.get());

    world.broadphase = std::make_shared<btDbvtBroadphase>();

    /// the default constraint solver. For parallel processing you can use a different solver (see
    /// Extras/BulletMultiThreaded)
    world.solver = std::make_shared<btSequentialImpulseConstraintSolver>();

    world.dynamics_world = std::make_shared<btDiscreteDynamicsWorld>(world.dispatcher.get(),
                                                                     world.broadphase.get(),
                                                                     world.solver.get(),
                                                                     world.collision_config.get());

    world.dynamics_world->setGravity(btVector3(0, -10, 0));

    return world;
}

auto to_bullet(const math::vec3& v) -> btVector3
{
    return {v.x, v.y, v.z};
}

auto from_bullet(const btVector3& v) -> math::vec3
{
    return {v.x(), v.y(), v.z()};
}


auto to_bullet(const math::quat& q) -> btQuaternion
{
    return {q.x, q.y, q.z, q.w};
}

auto from_bullet(const btQuaternion& q) -> math::quat
{
    math::quat r;
    r.x = q.x();
    r.y = q.y();
    r.z = q.z();
    r.w = q.w();
    return r;
}

} // namespace bullet

namespace ace
{

namespace
{
const uint8_t system_id = 1;

void wake_up(bullet::rigidbody& body)
{
    if(body.internal)
    {
        body.internal->activate(true);
    }
}

auto make_rigidbody_shape(physics_component& comp) -> std::shared_ptr<btCompoundShape>
{
    auto compound_shapes = comp.get_shapes();
    if(compound_shapes.empty())
    {
        return nullptr;
    }
    else
    {
        auto cp = std::make_shared<btCompoundShape>();
        for(const auto& s : compound_shapes)
        {
            if(hpp::holds_alternative<physics_box_shape>(s.shape))
            {
                const auto& shape = hpp::get<physics_box_shape>(s.shape);
                auto half_extends = shape.extends * 0.5f;

                btBoxShape* box_shape = new btBoxShape({half_extends.x, half_extends.y, half_extends.z});

                btTransform localTransform;
                localTransform.setOrigin(bullet::to_bullet(shape.center));
                cp->addChildShape(localTransform, box_shape);
            }
            else if(hpp::holds_alternative<physics_sphere_shape>(s.shape))
            {
                const auto& shape = hpp::get<physics_sphere_shape>(s.shape);

                btSphereShape* sphere_shape = new btSphereShape(shape.radius);
                btTransform localTransform;
                localTransform.setOrigin(bullet::to_bullet(shape.center));
                cp->addChildShape(localTransform, sphere_shape);
            }
            else if(hpp::holds_alternative<physics_capsule_shape>(s.shape))
            {
                const auto& shape = hpp::get<physics_capsule_shape>(s.shape);

                btCapsuleShape* capsule_shape = new btCapsuleShape(shape.radius, shape.length);
                btTransform localTransform;
                localTransform.setOrigin(bullet::to_bullet(shape.center));
                cp->addChildShape(localTransform, capsule_shape);
            }
            else if(hpp::holds_alternative<physics_cylinder_shape>(s.shape))
            {
                const auto& shape = hpp::get<physics_cylinder_shape>(s.shape);

                btVector3 half_extends(shape.radius, shape.length, shape.radius);
                btCylinderShape* cylinder_shape = new btCylinderShape(half_extends);
                btTransform localTransform;
                localTransform.setOrigin(bullet::to_bullet(shape.center));
                cp->addChildShape(localTransform, cylinder_shape);
            }
        }

        return cp;
    }
}

void update_rigidbody_shape(bullet::rigidbody& body, physics_component& comp)
{
    auto shape = make_rigidbody_shape(comp);

    body.internal->setCollisionShape(shape.get());
    body.internal_shape = shape;
}

void update_rigidbody_kind(bullet::rigidbody& body, physics_component& comp)
{
    if(comp.is_kinematic())
    {
        body.internal->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
    }
    else
    {
        body.internal->setCollisionFlags(btCollisionObject::CF_DYNAMIC_OBJECT);
    }
}

void update_rigidbody_mass_and_inertia(bullet::rigidbody& body, physics_component& comp)
{
    btScalar mass(0);
    btVector3 localInertia(0, 0, 0);
    if(!comp.is_kinematic())
    {
        auto shape = body.internal->getCollisionShape();
        if(shape)
        {
            mass = comp.get_mass();
            shape->calculateLocalInertia(mass, localInertia);
        }
    }
    body.internal->setMassProps(mass, localInertia);
}

void make_rigidbody(bullet::world& world, entt::handle entity, physics_component& comp)
{
    auto& body = entity.emplace<bullet::rigidbody>();

    body.internal = std::make_shared<btRigidBody>(comp.get_mass(), nullptr, nullptr);

    update_rigidbody_kind(body, comp);
    update_rigidbody_shape(body, comp);
    update_rigidbody_mass_and_inertia(body, comp);

    world.dynamics_world->addRigidBody(body.internal.get());
}

void destroy_phyisics_body(bullet::world& world, physics_component& comp)
{
    auto owner = comp.get_owner();
    auto body = owner.try_get<bullet::rigidbody>();

    if(body && body->internal)
    {
        world.dynamics_world->removeRigidBody(body->internal.get());
        body->internal = {};
        body->internal_shape = {};
    }

    owner.remove<bullet::rigidbody>();
}

void recreate_phyisics_body(bullet::world& world, physics_component& comp, bool force = false)
{
    bool is_kind_dirty = comp.is_property_dirty(physics_property::kind);
    bool needs_recreation = force || comp.are_all_properties_dirty();

    auto owner = comp.get_owner();

    if(needs_recreation)
    {
        destroy_phyisics_body(world, comp);
        make_rigidbody(world, owner, comp);
    }
    else
    {
        auto& body = owner.get<bullet::rigidbody>();

        if(comp.is_property_dirty(physics_property::mass) || is_kind_dirty)
        {
            update_rigidbody_mass_and_inertia(body, comp);
        }
        if(comp.is_property_dirty(physics_property::gravity) || is_kind_dirty)
        {
        }
        if(comp.is_property_dirty(physics_property::material) || is_kind_dirty)
        {
        }
        if(comp.is_property_dirty(physics_property::shape) || is_kind_dirty)
        {
            update_rigidbody_shape(body, comp);
            update_rigidbody_mass_and_inertia(body, comp);
        }
        if(is_kind_dirty)
        {
            update_rigidbody_kind(body, comp);
        }

        if(!comp.is_kinematic())
        {
            if(comp.are_any_properties_dirty())
            {
                wake_up(body);
            }
        }
    }

    comp.set_dirty(system_id, false);
}

void sync_transforms(physics_component& comp, const math::transform& transform)
{
    auto owner = comp.get_owner();
    auto& body = owner.get<bullet::rigidbody>();

    if(!body.internal)
    {
        return;
    }

    const auto& p = transform.get_position();
    const auto& q = transform.get_rotation();
    const auto& s = transform.get_scale();

    btTransform btTrans(bullet::to_bullet(q), bullet::to_bullet(p));
    body.internal->setWorldTransform(btTrans);

    if(body.internal_shape)
    {
        auto scale = bullet::from_bullet(body.internal_shape->getLocalScaling());

        if(math::any(math::epsilonNotEqual(scale, s, math::epsilon<float>())))
        {
            body.internal_shape->setLocalScaling(bullet::to_bullet(s));
        }
    }

    wake_up(body);
}

auto sync_transforms(physics_component& comp, math::transform& transform) -> bool
{
    auto owner = comp.get_owner();
    auto& body = owner.get<bullet::rigidbody>();

    if(!body.internal)
    {
        return false;
    }

    const auto& btTrans = body.internal->getWorldTransform();
    transform.set_position(bullet::from_bullet(btTrans.getOrigin()));
    transform.set_rotation(bullet::from_bullet(btTrans.getRotation()));

    return true;
}

void to_physics(bullet::world& world, transform_component& transform, physics_component& comp)
{
    bool transform_dirty = transform.is_dirty(system_id);
    bool rigidbody_dirty = comp.is_dirty(system_id);

    if(rigidbody_dirty)
    {
        recreate_phyisics_body(world, comp);
    }

    if(transform_dirty || rigidbody_dirty)
    {
        sync_transforms(comp, transform.get_transform_global());
    }
}

void from_physics(transform_component& transform, physics_component& comp)
{
    auto transform_global = transform.get_transform_global();
    if(sync_transforms(comp, transform_global))
    {
        transform.set_transform_global(transform_global);
    }

    transform.set_dirty(system_id, false);
    comp.set_dirty(system_id, false);
}

} // namespace

void bullet_backend::on_create_component(entt::registry& r, const entt::entity e)
{
}
void bullet_backend::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& registry = *entity.registry();

    auto& rigidbody = entity.get<physics_component>();

    auto world = registry.ctx().find<bullet::world>();
    if(world)
    {
        destroy_phyisics_body(*world, rigidbody);
    }
}

void bullet_backend::on_apply_impulse(physics_component& comp, const math::vec3& impulse)
{
    auto owner = comp.get_owner();
    auto& registry = *owner.registry();

    auto& bbody = owner.get<bullet::rigidbody>();
    bbody.internal->applyCentralImpulse({impulse.x, impulse.y, impulse.z});
    wake_up(bbody);
}

void bullet_backend::on_apply_torque_impulse(physics_component& comp, const math::vec3& impulse)
{
    auto owner = comp.get_owner();
    auto& registry = *owner.registry();

    auto& bbody = owner.get<bullet::rigidbody>();
    bbody.internal->applyTorqueImpulse({impulse.x, impulse.y, impulse.z});
    wake_up(bbody);
}

void bullet_backend::on_clear_kinematic_velocities(physics_component& comp)
{
    if(comp.is_kinematic())
    {
        auto owner = comp.get_owner();
        auto& registry = *owner.registry();

        auto& bbody = owner.get<bullet::rigidbody>();
        bbody.internal->clearForces();
        bbody.internal->applyGravity();

        wake_up(bbody);
    }
}

void bullet_backend::on_play_begin(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    auto& world = registry.ctx().emplace<bullet::world>(bullet::create_dynamics_world());

    registry.on_construct<physics_component>().connect<&bullet_backend::on_create_component>();
    registry.on_destroy<physics_component>().connect<&bullet_backend::on_destroy_component>();

    registry.view<physics_component>().each(
        [&](auto e, auto&& comp)
        {
            recreate_phyisics_body(world, comp, true);
        });
}

void bullet_backend::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    auto& world = registry.ctx().get<bullet::world>();

    registry.view<physics_component>().each(
        [&](auto e, auto&& comp)
        {
            destroy_phyisics_body(world, comp);
        });

    registry.on_construct<physics_component>().disconnect<&bullet_backend::on_create_component>();
    registry.on_destroy<physics_component>().disconnect<&bullet_backend::on_destroy_component>();

    registry.ctx().erase<bullet::world>();
}

void bullet_backend::on_pause(rtti::context& ctx)
{

}

void bullet_backend::on_resume(rtti::context& ctx)
{

}

void bullet_backend::on_skip_next_frame(rtti::context& ctx)
{
    delta_t step(1.0f / 60.0f);
    on_frame_update(ctx, step);
}

void bullet_backend::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;
    auto& world = registry.ctx().get<bullet::world>();

    if(os::key::is_pressed(os::key::code::space))
    {
        // update phyiscs spatial properties from transform
        registry.view<transform_component, physics_component>().each(
            [&](auto e, auto&& transform, auto&& rigidbody)
            {
                rigidbody.apply_impulse({0.0f, 100.0f * dt.count(), 0.0f});
            });
    }

    if(os::key::is_pressed(os::key::code::enter))
    {
        // update phyiscs spatial properties from transform
        registry.view<transform_component, physics_component>().each(
            [&](auto e, auto&& transform, auto&& rigidbody)
            {
                rigidbody.apply_torque_impulse({0.0f, 10.0f * dt.count(), 0.0f});
            });
    }

    // update phyiscs spatial properties from transform
    registry.view<transform_component, physics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            to_physics(world, transform, rigidbody);
        });

    // update physics
    world.dynamics_world->stepSimulation(dt.count());

    // update transform from phyiscs interpolated spatial properties
    registry.view<transform_component, physics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            from_physics(transform, rigidbody);
        });
}

void bullet_backend::draw_gizmo(physics_component& comp, const camera& cam, gfx::dd_raii& dd)
{

}


} // namespace ace
