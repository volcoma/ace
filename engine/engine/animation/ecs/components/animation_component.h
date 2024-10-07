#pragma once

#include <engine/ecs/components/basic_component.h>

#include <engine/animation/animation.h>
#include <engine/assets/asset_handle.h>
#include <engine/rendering/model.h>

#include <graphics/texture.h>
#include <hpp/variant.hpp>
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

using blend_easing_t = std::function<float(float)>;

struct blend_space_point
{
    std::vector<float> parameters;     // The parameter values for this point
    asset_handle<animation_clip> clip; // The animation clip associated with this point
};

class blend_space_def
{
public:
    using parameter_t = float;
    using parameters_t = std::vector<parameter_t>;

    // Add an animation clip to the blend space at specified parameter values
    void add_clip(const parameters_t& params, const asset_handle<animation_clip>& clip);

    // Compute the blending weights for the current parameters
    void compute_blend(const parameters_t& current_params,
                       std::vector<std::pair<asset_handle<animation_clip>, float>>& out_clips) const;

    // Get the number of parameters in the blend space
    auto get_parameter_count() const -> size_t;

private:
    std::vector<blend_space_point> points_;
    size_t parameter_count_{0};
};

struct animation_state
{
    asset_handle<animation_clip> clip{};
    animation_clip::seconds_t elapsed{};

    // Add blend space support
    std::shared_ptr<blend_space_def> blend_space{};
    std::vector<std::pair<asset_handle<animation_clip>, float>> blend_clips{};
    std::vector<animation_pose> blend_poses{};
};

struct blend_over_time
{
    animation_clip::seconds_t duration{};
    animation_clip::seconds_t elapsed{};

    auto get_progress() const -> float
    {
        // Compute the normalized blending time (clamped between 0 and 1)
        auto normalized_blend_time = static_cast<float>(elapsed.count() / duration.count());
        normalized_blend_time = std::clamp(normalized_blend_time, 0.0f, 1.0f);
        return normalized_blend_time;
    }
};

struct blend_over_param
{
    float param{};

    auto get_progress() const -> float
    {
        return param;
    }
};

struct blend_state
{
    blend_easing_t easing{math::linearInterpolation<float>};
    hpp::variant<hpp::monostate, blend_over_time, blend_over_param> state{};
};

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

    /**
     * @brief Blends to the animation over the specified time with the specified easing
     *
     * @param clip The animation to blend to.
     * @param duration The duration over which the blending must complete.
     * @param easing The easing function used.
     */

    void blend_to(const asset_handle<animation_clip>& clip,
                  seconds_t duration = seconds_t(0.3),
                  const blend_easing_t& easing = math::linearInterpolation<float>);

    void set_blend_space(const std::shared_ptr<blend_space_def>& blend_space);

    void set_blend_space_parameters(const std::vector<float>& params);
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
    struct animation_layer
    {
        bool is_valid() const
        {
            return state.clip || state.blend_space;
        }
        /// Current state
        animation_pose pose{};
        animation_state state{};
        std::vector<float> parameters;
    };


    void sample_animation(const animation_clip* anim_clip, seconds_t time, animation_pose& pose) const noexcept;
    auto compute_blend_factor(float normalized_blend_time) noexcept -> float;
    void update_state(seconds_t delta_time, animation_state& state);
    auto get_blend_progress() const -> float;
    auto update_pose(animation_layer& layer) -> bool;


    animation_layer current_layer_{};
    animation_layer target_layer_{};

    /// Blended state
    animation_pose blend_pose_{};
    blend_state blend_state_{};

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
