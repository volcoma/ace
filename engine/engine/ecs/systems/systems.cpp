#include "systems.h"

#include <engine/engine.h>
#include <engine/ecs/systems/bone_system.h>
#include <engine/ecs/systems/camera_system.h>
#include <engine/ecs/systems/reflection_probe_system.h>

namespace ace
{


void rendering_systems::on_frame_update(scene& scn, delta_t dt)
{

    auto& ctx = engine::context();
    ctx.get<camera_system>().on_frame_update(scn, dt);
    ctx.get<bone_system>().on_frame_update(scn, dt);
    ctx.get<reflection_probe_system>().on_frame_update(scn, dt);
}

} // namespace ace
