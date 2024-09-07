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

auto blend_poses(const pose_transform& pose1, const pose_transform& pose2, float factor) -> pose_transform
{
    pose_transform result_pose;

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

    return result_pose;
}

auto animation_player::set_animation(const asset_handle<animation_clip>& anim) -> bool
{
    if(current_animation_ == anim)
    {
        return false;
    }
    current_animation_ = anim;
    current_time_ = seconds_t(0);
    playing_ = false;
    paused_ = false;

    return true;
}

void animation_player::play()
{
    if(current_animation_)
    {
        playing_ = true;
        paused_ = false;
    }
}

void animation_player::pause()
{
    paused_ = true;
}

void animation_player::stop()
{
    playing_ = false;
    paused_ = false;
    current_time_ = seconds_t(0);
}

void animation_player::update(seconds_t delta_time, const update_callback_t& set_transform_callback)
{
    if(!current_animation_ || !is_playing())
    {
        return;
    }

    current_time_ += delta_time;

    auto current_animation = current_animation_.get();
    // Loop the animation
    if(current_time_ > current_animation->duration)
    {
        current_time_ = seconds_t(std::fmod(current_time_.count(), current_animation->duration.count()));
    }

    math::transform transform;
    for(const auto& channel : current_animation->channels)
    {
        math::vec3 position = interpolate(channel.position_keys, current_time_);
        math::quat rotation = interpolate(channel.rotation_keys, current_time_);
        math::vec3 scaling = interpolate(channel.scaling_keys, current_time_);

        // Compute the transformation matrix

        transform.set_position(position);
        transform.set_rotation(rotation);
        transform.set_scale(scaling);
        // Apply the transformation to the corresponding node
        set_transform_callback(channel.node_name, channel.node_index, transform);
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
