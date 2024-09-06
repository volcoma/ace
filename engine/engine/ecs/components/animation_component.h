#pragma once

#include "basic_component.h"

#include <engine/animation/animation.h>
#include <engine/assets/asset_handle.h>
#include <engine/rendering/model.h>

#include <graphics/texture.h>
#include <math/math.h>

namespace ace
{

// auto blend(const math::transform& lhs, const math::transform& rhs, float blendFactor) -> math::transform;

// auto blend_poses(const pose_transform& pose1, const pose_transform& pose2, float blendFactor) -> pose_transform;
/**
 * @brief Class responsible for playing animations on a skeletal mesh.
 *
 * This class handles the playback of animations, interpolating between keyframes
 * and applying the appropriate transformations to the nodes of a skeletal mesh.
 */
class animation_player
{
public:
    using seconds_t = animation_clip::seconds_t;
    using update_callback_t = std::function<void(const std::string&, size_t, const math::transform&)>;
    /**
     * @brief Sets the current animation to play and starts playback.
     *
     * @param anim The animation to play.
     */
    auto set_animation(const asset_handle<animation_clip>& anim) -> bool;

    /**
     * @brief Starts or resumes the animation playback.
     */
    void play();

    /**
     * @brief Pauses the animation playback.
     */
    void pause();

    /**
     * @brief Stops the animation playback and resets the time.
     */
    void stop();

    /**
     * @brief Updates the animation player, advancing the animation time and applying transformations.
     *
     * @param delta_time The time to advance the animation by.
     * @param set_transform_callback The callback function to set the transform of a node.
     */
    void update(seconds_t delta_time, const update_callback_t& set_transform_callback);

    /**
     * @brief Returns whether the animation is currently playing.
     *
     * @return True if the animation is playing, false otherwise.
     */
    auto is_playing() const -> bool;

    /**
     * @brief Returns whether the animation is currently paused.
     *
     * @return True if the animation is paused, false otherwise.
     */
    auto is_paused() const -> bool;

private:
    asset_handle<animation_clip> current_animation_{};
    seconds_t current_time_ = seconds_t(0);
    bool playing_ = false;
    bool paused_ = false;
};

struct animation_component : public component_crtp<animation_component>
{
    asset_handle<animation_clip> animation;
    animation_player player;
};

} // namespace ace
