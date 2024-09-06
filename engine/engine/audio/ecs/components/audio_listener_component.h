#pragma once
#include <engine/engine_export.h>

#include <engine/ecs/components/basic_component.h>
#include <audiopp/listener.h>
#include <math/math.h>

namespace ace
{

/**
 * @class audio_listener_component
 * @brief Class that contains core data for audio listeners.
 * There can only be one instance of it per scene.
 */
class audio_listener_component : public component_crtp<audio_listener_component>
{
public:
    /**
     * @brief Updates the audio listener with the given transform and delta time.
     * @param t The transform to update with.
     * @param dt The delta time.
     */
    void update(const math::transform& t, delta_t dt);

private:
    /**
     * @brief The audio listener object.
     */
    std::shared_ptr<audio::listener> listener_;
};

} // namespace ace
