#include "newton_backend.h"

#include <engine/defaults/defaults.h>
#include <engine/events.h>
#include <math/transform.hpp>

#include <engine/ecs/ecs.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/physics/ecs/components/physics_component.h>

// #include <dNewton/ndNewton.h>
#include <logging/logging.h>

namespace newton
{
struct rigidbody
{
    // ndBody* internal{};
    int* internal{};
};


}

namespace ace
{

namespace
{
const uint8_t system_id = 1;
void destroy_phyisics_body(/*ndWorld& world, */physics_component& rigidbody)
{
    auto owner = rigidbody.get_owner();
    auto& body = owner.get<newton::rigidbody>();

    if(body.internal)
    {
        // world.RemoveBody(body.internal);
        body.internal = nullptr;
    }

    owner.remove<newton::rigidbody>();
}

void recreate_phyisics_body(/*ndWorld& world, */physics_component& rigidbody, bool force = false)
{
    auto owner = rigidbody.get_owner();

    bool needs_recreation = force || rigidbody.is_property_dirty(physics_property::kind);

    if(needs_recreation)
    {
        auto& body = owner.emplace_or_replace<newton::rigidbody>();

        if(body.internal)
        {
            // world.RemoveBody(body.internal);
            body.internal = nullptr;
        }

        // ndMatrix matrix(ndGetIdentityMatrix());

        // ndShapeInstance nshape(new ndShapeBox(ndFloat32(1), ndFloat32(1), ndFloat32(1)));

        // if(rigidbody.is_kinematic())
        // {
        //     ndBodyKinematic* nbody = new ndBodyKinematic();
        //     nbody->SetCollisionShape(nshape);
        //     nbody->SetMatrix(matrix);
        //     nbody->SetMassMatrix(rigidbody.get_mass(), nshape);

        //     body.internal = nbody;

        // }
        // else
        // {
        //     ndBodyDynamic* nbody = new ndBodyDynamic();

        //     if(rigidbody.is_using_gravity())
        //     {
        //         nbody->SetNotifyCallback(new ndBodyNotify(ndBigVector(ndFloat32(0), ndFloat32(-9.81f), ndFloat32(0), ndFloat32(0))));
        //     }
        //     nbody->SetCollisionShape(nshape);
        //     nbody->SetMatrix(matrix);
        //     nbody->SetMassMatrix(rigidbody.get_mass(), nshape);
        //     nbody->SetDebugMaxLinearAndAngularIntegrationStep(ndPi, 2.0f);

        //     body.internal = nbody;

        // }
        //recreate_phyisics_entity(rigidbody);
        // ndShapeInstance planetshape(new ndShapeSphere(ndFloat32(6357000)));

        // ndBodyKinematic* staticbody = new ndBodyKinematic();
        // staticbody->SetCollisionShape(planetshape);
        // staticbody->SetMatrix(matrix);
        // staticbody->SetMassMatrix(ndFloat32(5.9722e+24), planetshape);
        // ndSharedPtr<ndBody> staticPtr(staticbody);
        // world.AddBody(staticPtr);

        // matrix.m_posit.m_y = 6357000 + 1021140;

        // ndBodyDynamic* movingbody = new ndBodyDynamic();
        // movingbody->SetNotifyCallback(new ndBodyNotify(ndBigVector(ndFloat32(0), ndFloat32(-9.81f), ndFloat32(0), ndFloat32(0))));
        // movingbody->SetCollisionShape(shapeinst);
        // movingbody->SetMatrix(matrix);
        // movingbody->SetMassMatrix(ndFloat32(10), shapeinst);
        // movingbody->SetDebugMaxLinearAndAngularIntegrationStep(ndPi, 2.0f);
        // ndSharedPtr<ndBody> movingPtr(movingbody);
        // world.AddBody(body.internal);
    }

    auto& body = owner.get<newton::rigidbody>();

    // update_def_mass(rigidbody, body.def);
    // update_def_kind(rigidbody, body.def);
    // update_def_shape(rigidbody, body.def);
    // update_def_material(rigidbody, body.def);
    // update_def_gravity(rigidbody, body.def);

    if(needs_recreation)
    {
        // edyn::make_rigidbody(entity, registry, body.def);
    }
    else
    {
        if(rigidbody.is_property_dirty(physics_property::mass))
        {
            // edyn::update_rigidbody_mass(entity, registry, body.def);
        }
        if(rigidbody.is_property_dirty(physics_property::gravity))
        {
            // edyn::update_rigidbody_gravity(entity, registry, body.def);
        }
        if(rigidbody.is_property_dirty(physics_property::material))
        {
            // edyn::update_rigidbody_material(entity, registry, body.def);
        }
        if(rigidbody.is_property_dirty(physics_property::shape))
        {
            // edyn::update_rigidbody_shape(entity, registry, body.def);
        }
    }

    rigidbody.set_dirty(system_id, false);
}


void sync_transforms(physics_component& rigidbody, const math::transform& transform)
{
    auto owner = rigidbody.get_owner();
    auto& body = owner.get<newton::rigidbody>();

    if(!body.internal)
    {
        return;
    }

    const auto& p = transform.get_position();
    const auto& q = transform.get_rotation();

    // ndQuaternion ndquat;
    // ndquat.SetX(q.x);
    // ndquat.SetY(q.y);
    // ndquat.SetZ(q.z);
    // ndquat.SetW(q.w);

    // ndVector ndpos;
    // ndpos.SetX(p.x);
    // ndpos.SetY(p.y);
    // ndpos.SetZ(p.z);
    // ndpos.SetW(ndFloat32(1.0f));
    // body.internal->SetMatrix(ndCalculateMatrix(ndquat, ndpos));
}

auto sync_transforms(physics_component& rigidbody, math::transform& transform) -> bool
{
    auto owner = rigidbody.get_owner();
    auto& body = owner.get<newton::rigidbody>();

    if(!body.internal)
    {
        return false;
    }

    // auto ndpos = body.internal->GetPosition();
    // auto ndquat = body.internal->GetRotation();

    // math::vec3 p;
    // p.x = ndpos.GetX();
    // p.y = ndpos.GetY();
    // p.z = ndpos.GetZ();
    // transform.set_position(p);


    // math::quat q;
    // q.x = ndquat.GetX();
    // q.y = ndquat.GetY();
    // q.z = ndquat.GetZ();
    // q.w = ndquat.GetW();
    // transform.set_rotation(q);


    return true;
}

void to_physics(transform_component& transform, physics_component& rigidbody)
{
    bool transform_dirty = transform.is_dirty(system_id);
    bool rigidbody_dirty = rigidbody.is_dirty(system_id);

    if(rigidbody_dirty)
    {
        //recreate_phyisics_body(rigidbody);
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

} // namespace

void newton_backend::on_create_component(entt::registry& r, const entt::entity e)
{
}
void newton_backend::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& registry = *entity.registry();

    auto& rigidbody = entity.get<physics_component>();

    // auto world = registry.ctx().find<ndWorld>();
    // if(world)
    // {
    //     destroy_phyisics_body(*world, rigidbody);
    // }
}

void newton_backend::on_apply_impulse(physics_component& comp, const math::vec3& impulse)
{
    // auto owner = comp.get_owner();
    // auto& registry = *owner.registry();

    // auto& ebody = owner.get<edyn::rigidbody>();
    // edyn::rigidbody_apply_impulse(registry, ebody.internal, {impulse.x, impulse.y, impulse.z}, edyn::vector3_zero);

    // edyn::wake_up_entity(registry, ebody.internal);
}

void newton_backend::on_apply_torque_impulse(physics_component& comp, const math::vec3& impulse)
{
    // auto owner = comp.get_owner();
    // auto& registry = *owner.registry();

    // auto& ebody = owner.get<edyn::rigidbody>();

    // edyn::rigidbody_apply_torque_impulse(registry, ebody.internal, {impulse.x, impulse.y, impulse.z});

    // edyn::wake_up_entity(registry, ebody.internal);
}

void newton_backend::on_clear_kinematic_velocities(physics_component& comp)
{
    if(comp.is_kinematic())
    {
        // auto owner = comp.get_owner();
        // auto& registry = *owner.registry();

        // auto& ebody = owner.get<edyn::rigidbody>();

        // auto [lvel, avel] = ebody.internal.try_get<edyn::linvel, edyn::angvel>();

        // if(lvel)
        // {
        //     *lvel = edyn::vector3_zero;
        //     ebody.internal.patch<edyn::linvel>();
        // }

        // if(avel)
        // {
        //     *avel = edyn::vector3_zero;
        //     ebody.internal.patch<edyn::linvel>();
        // }

        // edyn::wake_up_entity(registry, ebody.internal);
    }
}

void newton_backend::on_play_begin(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();
    auto& registry = *scn.registry;

    // auto& world = registry.ctx().emplace<ndWorld>();

    registry.view<physics_component>().each(
        [&](auto e, auto&& comp)
        {
            recreate_phyisics_body(/*world, */comp, true);
        });
}

void newton_backend::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    // auto& world = registry.ctx().emplace<ndWorld>();

    registry.view<physics_component>().each(
        [&](auto e, auto&& comp)
        {
            destroy_phyisics_body(/*world, */comp);
        });

    // registry.ctx().erase<ndWorld>();
}

void newton_backend::on_pause(rtti::context& ctx)
{
    // auto& ec = ctx.get<ecs>();
    // auto& registry = *ec.get_scene().registry;

    // edyn::set_paused(registry, true);
}

void newton_backend::on_resume(rtti::context& ctx)
{
    // auto& ec = ctx.get<ecs>();
    // auto& registry = *ec.get_scene().registry;

    // edyn::set_paused(registry, false);
}

void newton_backend::on_skip_next_frame(rtti::context& ctx)
{
    // auto& ec = ctx.get<ecs>();
    // auto& registry = *ec.get_scene().registry;

    // edyn::step_simulation(registry);
}

void newton_backend::on_frame_update(rtti::context& ctx, delta_t dt)
{
    auto& ec = ctx.get<ecs>();
    auto& registry = *ec.get_scene().registry;

    // auto& world = registry.ctx().get<ndWorld>();
                                //     world.Sync();
    // if(os::key::is_pressed(os::key::code::space))
    // {
    //     // update phyiscs spatial properties from transform
    //     registry.view<transform_component, physics_component>().each(
    //         [&](auto e, auto&& transform, auto&& rigidbody)
    //         {
    //             rigidbody.apply_impulse({0.0f, 2.0f, 0.0f});
    //         });
    // }

    // if(os::key::is_pressed(os::key::code::enter))
    // {
    //     // update phyiscs spatial properties from transform
    //     registry.view<transform_component, physics_component>().each(
    //         [&](auto e, auto&& transform, auto&& rigidbody)
    //         {
    //             rigidbody.apply_impulse({1.0f, 1.0f, 0.0f});
    //         });
    // }

    // update phyiscs spatial properties from transform
    registry.view<transform_component, physics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            to_physics(transform, rigidbody);
        });

    // update physics
    // world.Update(1.0f / 60.0f);

    // update transform from phyiscs interpolated spatial properties
    registry.view<transform_component, physics_component>().each(
        [&](auto e, auto&& transform, auto&& rigidbody)
        {
            from_physics(transform, rigidbody);
        });
}

} // namespace ace
