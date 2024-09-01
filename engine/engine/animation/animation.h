#pragma once
#include <math/math.h>

#include <chrono>
#include <string>
#include <vector>

namespace ace
{

/**
 * @brief Struct representing a node animation.
 *
 * This struct contains animation data for a specific node, including position, rotation, and scaling keys.
 */
struct node_animation
{
    using seconds_t = std::chrono::duration<float>;

    /**
     * @brief Struct representing a keyframe for animation.
     *
     * @tparam T The type of value stored in the keyframe.
     */
    template<typename T>
    struct key
    {
        /// The time of the keyframe.
        seconds_t time = seconds_t(0);

        /// The value of the keyframe.
        T value = {};
    };

    /// The name of the node affected by this animation. The node must exist and it must be unique.
    std::string node_name;

    size_t node_index{};

    /// The position keys of this animation channel. Positions are specified as 3D vector.
    std::vector<key<math::vec3>> position_keys;

    /// The rotation keys of this animation channel. Rotations are given as quaternions.
    std::vector<key<math::quat>> rotation_keys;

    /// The scaling keys of this animation channel. Scalings are specified as 3D vector.
    std::vector<key<math::vec3>> scaling_keys;
};

/**
 * @brief Struct representing an animation.
 *
 * This struct contains data for an entire animation, including the name, duration, and node animation channels.
 */
struct animation
{
    using seconds_t = node_animation::seconds_t;

    /// The name of the animation. Usually empty if the modeling package supports only a single animation channel.
    std::string name;

    /// Duration of the animation in seconds.
    seconds_t duration = seconds_t(0);

    /// The node animation channels. Each channel affects a single node.
    std::vector<node_animation> channels;
};

} // namespace ace
