#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <engine/ecs/scene.h>

namespace ace
{
class rendering_systems
{
public:
    static void on_frame_update(scene& scn, delta_t dt);

};
} // namespace ace
