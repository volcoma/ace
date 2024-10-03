#pragma once

#include <engine/ecs/components/basic_component.h>

#include <engine/animation/animation.h>
#include <engine/assets/asset_handle.h>
#include <engine/rendering/model.h>

#include <graphics/texture.h>
#include <math/math.h>

namespace ace
{

struct animation_pose
{
    struct node
    {
        size_t index{};
        math::transform transform{};
    };

    std::vector<node> nodes;
};

auto blend(const math::transform& lhs, const math::transform& rhs, float factor) -> math::transform;

void blend_poses(const pose_transform& pose1, const pose_transform& pose2, float factor, pose_transform& result_pose);

void blend_poses(const animation_pose& pose1, const animation_pose& pose2, float factor, animation_pose& result_pose);

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
    using update_callback_t = std::function<void(/*const std::string&, */ size_t, const math::transform&)>;
    using easing_t = std::function<float(float)>;
    /**
     * @brief Blends to the animation
     *
     * @param anim The animation to play.
     */

    void blend_to_animation(const asset_handle<animation_clip>& new_animation,
                            seconds_t blending_duration = seconds_t(0.5),
                            const easing_t& easing = math::linearInterpolation<float>);

    /**
     * @brief Starts or resumes the animation playback.
     */
    auto play() -> bool;

    /**
     * @brief Pauses the animation playback.
     */
    void pause();

    /**
     * @brief Resumes the animation playback.
     */
    void resume();

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
    void update(seconds_t delta_time, const update_callback_t& set_transform_callback, bool force = false);

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
    void sample_animation(const animation_clip* anim_clip, seconds_t time, animation_pose& pose) const noexcept;
    auto compute_blend_factor() noexcept -> float;
    void update_current(seconds_t delta_time);
    void update_target(seconds_t delta_time);
    // Existing private members...
    asset_handle<animation_clip> current_animation_{};
    seconds_t current_time_{};

    // Blending parameters
    asset_handle<animation_clip> target_animation_{};
    seconds_t target_time_{};

    seconds_t blending_duration_{};
    seconds_t blending_time_elapsed_{};

    // Poses for blending
    animation_pose current_pose_{};
    animation_pose target_pose_{};
    animation_pose blended_pose_{};

    // Easing function for blending
    easing_t easing_function_{math::linearInterpolation<float>};

    bool playing_{};
    bool paused_{};
};

class animation_component : public component_crtp<animation_component>
{
public:
    enum class culling_mode : uint8_t
    {
        always_animate,
        renderer_based,
    };

    /**
     * @brief Sets whether the animation should autoplay.
     * @param on True to autoplay, false otherwise.
     */
    void set_autoplay(bool on);

    /**
     * @brief Gets whether the animation is set to autoplay.
     * @return True if autoplay is enabled, false otherwise.
     */
    auto get_autoplay() const -> bool;

    void set_animation(const asset_handle<animation_clip>& animation);
    auto get_animation() const -> const asset_handle<animation_clip>&;

    void set_culling_mode(const culling_mode& animation);
    auto get_culling_mode() const -> const culling_mode&;

    auto get_player() const -> const animation_player&;
    auto get_player() -> animation_player&;

private:
    asset_handle<animation_clip> animation_;

    animation_player player_;

    culling_mode culling_mode_{culling_mode::always_animate};
    bool auto_play_ = true;
};

} // namespace ace
