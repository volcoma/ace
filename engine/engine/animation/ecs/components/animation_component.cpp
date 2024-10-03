#include "animation_component.h"

namespace ace
{

namespace
{
/**
 * @brief Interpolates between keyframes to find the appropriate value at the current time.
 *  * @tparam T The type of value being interpolated (e.g., vec3 or quat).
 * @param keys The list of keyframes.
 * @param time The current animation time.
 * @return The interpolated value.
 */
template<typename T>
auto interpolate(const std::vector<animation_channel::key<T>>& keys, animation_player::seconds_t time) -> T
{
    if(keys.empty())
    {
        return {}; // Return default value if there are no keys
    }

    // Do binary search for keyframe
    int high = (int)keys.size(), low = -1;
    while(high - low > 1)
    {
        int probe = (high + low) / 2;
        if(keys[probe].time < time)
        {
            low = probe;
        }
        else
        {
            high = probe;
        }
    }

    if(low == -1)
    {
        // Before first key, return first key
        return keys.front().value;
    }

    if(high == (int)keys.size())
    {
        // Beyond last key, return last key
        return keys.back().value;
    }

    const auto& key1 = keys[low];
    const auto& key2 = keys[low + 1];

    // Compute the interpolation factor (0.0 to 1.0)
    float factor = (time.count() - key1.time.count()) / (key2.time.count() - key1.time.count());

    // Perform the interpolation
    if constexpr(std::is_same_v<T, math::vec3>)
    {
        return math::lerp(key1.value, key2.value, factor);
    }
    else if constexpr(std::is_same_v<T, math::quat>)
    {
        return math::slerp(key1.value, key2.value, factor);
    }

    return {};
}

} // namespace

auto blend(const math::transform& lhs, const math::transform& rhs, float factor) -> math::transform
{
    math::transform result;
    result.set_translation(math::lerp(lhs.get_translation(), rhs.get_translation(), factor));
    result.set_rotation(math::slerp(lhs.get_rotation(), rhs.get_rotation(), factor));
    result.set_scale(math::lerp(lhs.get_scale(), rhs.get_scale(), factor));
    return result;
}

void blend_poses(const pose_transform& pose1, const pose_transform& pose2, float factor, pose_transform& result_pose)
{
    // Determine the maximum number of transforms between both poses
    size_t max_transforms = std::max(pose1.transforms.size(), pose2.transforms.size());

    // Resize the result pose to hold the maximum number of transforms
    result_pose.transforms.resize(max_transforms);

    // Iterate through each bone transform and blend them
    for(size_t i = 0; i < max_transforms; ++i)
    {
        if(i < pose1.transforms.size() && i < pose2.transforms.size())
        {
            // Both poses have this bone, so blend them
            result_pose.transforms[i] = blend(pose1.transforms[i], pose2.transforms[i], factor);
        }
        else if(i < pose1.transforms.size())
        {
            // Only pose1 has this bone, use pose1's transform
            result_pose.transforms[i] = pose1.transforms[i];
        }
        else if(i < pose2.transforms.size())
        {
            // Only pose2 has this bone, use pose2's transform
            result_pose.transforms[i] = pose2.transforms[i];
        }
    }
}

void blend_poses(const animation_pose& pose1, const animation_pose& pose2, float factor, animation_pose& result_pose)
{
    // Determine the maximum number of transforms between both poses
    size_t max_transforms = std::max(pose1.nodes.size(), pose2.nodes.size());

    // Resize the result pose to hold the maximum number of transforms
    result_pose.nodes.resize(max_transforms);

    // Iterate through each bone transform and blend them
    for(size_t i = 0; i < max_transforms; ++i)
    {
        if(i < pose1.nodes.size() && i < pose2.nodes.size())
        {
            // Both poses have this bone, so blend them
            result_pose.nodes[i].index = pose1.nodes[i].index;
            result_pose.nodes[i].transform = blend(pose1.nodes[i].transform, pose2.nodes[i].transform, factor);
        }
        else if(i < pose1.nodes.size())
        {
            // Only pose1 has this bone, use pose1's transform
            result_pose.nodes[i] = pose1.nodes[i];
        }
        else if(i < pose2.nodes.size())
        {
            // Only pose2 has this bone, use pose2's transform
            result_pose.nodes[i] = pose2.nodes[i];
        }
    }
}

void animation_player::blend_to_animation(const asset_handle<animation_clip>& new_animation,
                                          seconds_t blending_duration,
                                          const easing_t& easing)
{
    if(new_animation)
    {
        if(target_animation_ == new_animation)
        {
            return;
        }

        if(current_animation_ == new_animation)
        {
            return;
        }

        blending_duration_ = blending_duration;
    }
    else
    {
        if(current_animation_)
        {
            current_animation_ = {};
            current_time_ = {};
        }
    }

    // Set blending parameters
    target_animation_ = new_animation;
    blending_time_elapsed_ = seconds_t(0);

    // Reset target animation time
    // Optionally, you can start from a specific time
    target_time_ = seconds_t(0);

    easing_function_ = easing;
}

auto animation_player::play() -> bool
{
    if(playing_)
    {
        return false;
    }
    playing_ = true;
    paused_ = false;

    return true;
}

void animation_player::pause()
{
    paused_ = true;
}

void animation_player::resume()
{
    paused_ = false;
}

void animation_player::stop()
{
    playing_ = false;
    paused_ = false;
    current_time_ = seconds_t(0);
}

void animation_player::update(seconds_t delta_time, const update_callback_t& set_transform_callback, bool force)
{
    if((!current_animation_ && !target_animation_) || (!force && !is_playing()))
    {
        return;
    }

    // Update times
    if(playing_ && !paused_)
    {
        update_current(delta_time);

        update_target(delta_time);
    }

    // Update current pose
    if(current_animation_)
    {
        sample_animation(current_animation_.get().get(), current_time_, current_pose_);
    }
    animation_pose* final_pose = &current_pose_;

    // Update target pose if blending
    if(target_animation_)
    {
        sample_animation(target_animation_.get().get(), target_time_, target_pose_);

        // Compute blending factor
        float blend_factor = compute_blend_factor();

        // Blend poses
        blend_poses(current_pose_, target_pose_, blend_factor, blended_pose_);
        final_pose = &blended_pose_;

        // Check if blending is finished
        if(blending_time_elapsed_ >= blending_duration_)
        {
            // Switch to target animation
            current_animation_ = target_animation_;
            current_time_ = target_time_;
            target_animation_ = {};
            target_time_ = {};
        }
    }

    // Apply the final pose using the callback
    for(const auto& node : final_pose->nodes)
    {
        set_transform_callback(/* node name , */ node.index, node.transform);
    }
}

void animation_player::update_current(seconds_t delta_time)
{
    if(current_animation_)
    {
        current_time_ += delta_time;
        auto current_anim = current_animation_.get();
        if(current_time_ > current_anim->duration)
        {
            current_time_ = seconds_t(std::fmod(current_time_.count(), current_anim->duration.count()));
        }
    }
}

void animation_player::update_target(seconds_t delta_time)
{
    if(target_animation_)
    {
        blending_time_elapsed_ += delta_time;
        target_time_ += delta_time;
        auto target_anim = target_animation_.get();
        if(target_time_ > target_anim->duration)
        {
            target_time_ = seconds_t(std::fmod(target_time_.count(), target_anim->duration.count()));
        }
    }
}

auto animation_player::compute_blend_factor() noexcept -> float
{
    float blend_factor = 0.0f;

    // Compute the normalized blending time (clamped between 0 and 1)
    auto normalized_blend_time = static_cast<float>(blending_time_elapsed_.count() / blending_duration_.count());
    normalized_blend_time = std::clamp(normalized_blend_time, 0.0f, 1.0f);

    // Apply the easing function
    blend_factor = easing_function_(normalized_blend_time);

    // Check if blending is complete
    if(normalized_blend_time >= 1.0f)
    {
        // Blending complete
        blend_factor = 1.0f;
    }

    return blend_factor;
}

void animation_player::sample_animation(const animation_clip* anim_clip,
                                        seconds_t time,
                                        animation_pose& pose) const noexcept
{
    pose.nodes.clear();
    pose.nodes.reserve(anim_clip->channels.size());

    for(const auto& channel : anim_clip->channels)
    {
        math::vec3 position = interpolate(channel.position_keys, time);
        math::quat rotation = interpolate(channel.rotation_keys, time);
        math::vec3 scaling = interpolate(channel.scaling_keys, time);

        auto& node = pose.nodes.emplace_back();
        node.index = channel.node_index;

        node.transform.set_position(position);
        node.transform.set_rotation(rotation);
        node.transform.set_scale(scaling);
    }
}

auto animation_player::is_playing() const -> bool
{
    return playing_ && !paused_;
}

auto animation_player::is_paused() const -> bool
{
    return paused_;
}

void animation_component::set_animation(const asset_handle<animation_clip>& animation)
{
    animation_ = animation;
}
auto animation_component::get_animation() const -> const asset_handle<animation_clip>&
{
    return animation_;
}

void animation_component::set_autoplay(bool on)
{
    auto_play_ = on;
}

auto animation_component::get_autoplay() const -> bool
{
    return auto_play_;
}

void animation_component::set_culling_mode(const culling_mode& mode)
{
    culling_mode_ = mode;
}

auto animation_component::get_culling_mode() const -> const culling_mode&
{
    return culling_mode_;
}

auto animation_component::get_player() const -> const animation_player&
{
    return player_;
}
auto animation_component::get_player() -> animation_player&
{
    return player_;
}

} // namespace ace
