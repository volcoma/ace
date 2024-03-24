#pragma once

#include <engine/ecs/components/basic_component.h>

#include <audiopp/listener.h>

#include <math/math.h>

namespace ace
{
//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : audio_listener_component (Class)
/// <summary>
/// Class that contains core data for audio listeners.
/// There can only be one instance of it per scene.
/// </summary>
//-----------------------------------------------------------------------------
class audio_listener_component  : public component_crtp<audio_listener_component >
{
public:
    void update(const math::transform& t, delta_t dt);
private:
    //-------------------------------------------------------------------------
    // Private Member Variables.
    //-------------------------------------------------------------------------
    std::shared_ptr<audio::listener> listener_;
};

} // namespace ace
