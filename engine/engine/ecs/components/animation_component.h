#pragma once

#include "basic_component.h"

#include <engine/animation/animation.h>
#include <engine/assets/asset_handle.h>

#include <graphics/texture.h>
#include <math/math.h>

namespace ace
{

/**
 * @brief Class responsible for playing animations on a skeletal mesh.
 *
 * This class handles the playback of animations, interpolating between keyframes
 * and applying the appropriate transformations to the nodes of a skeletal mesh.
 */
class animation_player
{
public:
    using seconds_t = animation::seconds_t;

    /**
     * @brief Sets the current animation to play and starts playback.
     *
     * @param anim The animation to play.
     */
    auto set_animation(const asset_handle<animation>& anim) -> bool
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

    /**
     * @brief Starts or resumes the animation playback.
     */
    void play()
    {
        if(current_animation_)
        {
            playing_ = true;
            paused_ = false;
        }
    }

    /**
     * @brief Pauses the animation playback.
     */
    void pause()
    {
        paused_ = true;
    }

    /**
     * @brief Stops the animation playback and resets the time.
     */
    void stop()
    {
        playing_ = false;
        paused_ = false;
        current_time_ = seconds_t(0);
    }

    /**
     * @brief Updates the animation player, advancing the animation time and applying transformations.
     *
     * @param delta_time The time to advance the animation by.
     * @param set_transform_callback The callback function to set the transform of a node.
     */
    void update(seconds_t delta_time,
                const std::function<void(const std::string&, size_t, const math::transform&)>& set_transform_callback)
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

    /**
     * @brief Returns whether the animation is currently playing.
     *
     * @return True if the animation is playing, false otherwise.
     */
    auto is_playing() const -> bool
    {
        return playing_ && !paused_;
    }

    /**
     * @brief Returns whether the animation is currently paused.
     *
     * @return True if the animation is paused, false otherwise.
     */
    auto is_paused() const -> bool
    {
        return paused_;
    }

private:
    asset_handle<animation> current_animation_{};
    seconds_t current_time_ = seconds_t(0);
    bool playing_ = false;
    bool paused_ = false;

    /**
     * @brief Interpolates between keyframes to find the appropriate value at the current time.
     *
     * @tparam T The type of value being interpolated (e.g., vec3 or quat).
     * @param keys The list of keyframes.
     * @param time The current animation time.
     * @return The interpolated value.
     */
    template<typename T>
    auto interpolate(const std::vector<node_animation::key<T>>& keys, seconds_t time) -> T
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
};

struct animation_component : public component_crtp<animation_component>
{
    asset_handle<ace::animation> animation;
    animation_player player;

};

} // namespace ace
