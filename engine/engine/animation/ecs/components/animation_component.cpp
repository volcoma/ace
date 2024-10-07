#include "animation_component.h"
#include <hpp/utility/overload.hpp>
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

void blend_poses(const std::vector<animation_pose>& poses,
                 const std::vector<float>& weights,
                 animation_pose& result_pose)
{
    if(poses.empty() || weights.empty())
    {
        return;
    }

    // Initialize result_pose with zeros
    result_pose.nodes.clear();
    result_pose.nodes.resize(poses[0].nodes.size());

    // Ensure all poses have the same number of nodes
    for(size_t i = 0; i < result_pose.nodes.size(); ++i)
    {
        math::transform blended_transform;
        float total_weight = 0.0f;

        for(size_t j = 0; j < poses.size(); ++j)
        {
            if(i < poses[j].nodes.size())
            {
                const auto& node = poses[j].nodes[i];
                blended_transform = blend(blended_transform, node.transform, weights[j] / (total_weight + weights[j]));
                total_weight += weights[j];
            }
        }

        result_pose.nodes[i].index = poses[0].nodes[i].index; // Assuming same indices
        result_pose.nodes[i].transform = blended_transform;
    }
}

void blend_space_def::add_clip(const parameters_t& params, const asset_handle<animation_clip>& clip)
{
    points_.emplace_back(blend_space_point{params, clip});
    parameter_count_ = params.size(); // Ensure all points have the same number of parameters
}

void blend_space_def::compute_blend(const parameters_t& current_params,
                                    std::vector<std::pair<asset_handle<animation_clip>, float>>& out_clips) const
{
    // Clear the output vector
    out_clips.clear();

    // For simplicity, we'll handle a 2D blend space with bilinear interpolation
    if(parameter_count_ != 2)
    {
        // Implement support for other dimensions as needed
        return;
    }

    // Find the four closest points for bilinear interpolation
    // This involves finding the rectangle (grid cell) that the current parameters fall into

    // Collect all parameter values along each axis
    std::set<float> param0_values;
    std::set<float> param1_values;
    for(const auto& point : points_)
    {
        param0_values.insert(point.parameters[0]);
        param1_values.insert(point.parameters[1]);
    }

    // Convert sets to vectors for indexing
    std::vector<float> param0_vector(param0_values.begin(), param0_values.end());
    std::vector<float> param1_vector(param1_values.begin(), param1_values.end());

    // Find indices along each axis
    auto find_index = [](const std::vector<float>& values, float param) -> size_t
    {
        for(size_t i = 0; i < values.size() - 1; ++i)
        {
            if(param >= values[i] && param <= values[i + 1])
            {
                return i;
            }
        }
        return values.size() - 2; // Return last index if beyond range
    };

    size_t index0 = find_index(param0_vector, current_params[0]);
    size_t index1 = find_index(param1_vector, current_params[1]);

    // Get the parameter values at the grid corners
    float p00 = param0_vector[index0];
    float p01 = param0_vector[index0 + 1];
    float p10 = param1_vector[index1];
    float p11 = param1_vector[index1 + 1];

    // Collect the four corner points
    std::array<const blend_space_point*, 4> corner_points = {nullptr, nullptr, nullptr, nullptr};

    for(const auto& point : points_)
    {
        const auto& params = point.parameters;
        if(params[0] == p00 && params[1] == p10)
            corner_points[0] = &point; // Bottom-left
        if(params[0] == p01 && params[1] == p10)
            corner_points[1] = &point; // Bottom-right
        if(params[0] == p00 && params[1] == p11)
            corner_points[2] = &point; // Top-left
        if(params[0] == p01 && params[1] == p11)
            corner_points[3] = &point; // Top-right
    }

    // Ensure all corner points are found
    for(const auto* cp : corner_points)
    {
        if(!cp)
            return; // Cannot interpolate without all corner points
    }

    // Compute interpolation factors
    float tx = (current_params[0] - p00) / (p01 - p00);
    float ty = (current_params[1] - p10) / (p11 - p10);

    // Compute weights
    float w_bl = (1 - tx) * (1 - ty); // Bottom-left
    float w_br = tx * (1 - ty);       // Bottom-right
    float w_tl = (1 - tx) * ty;       // Top-left
    float w_tr = tx * ty;             // Top-right

    // Output the clips and their weights
    out_clips.emplace_back(corner_points[0]->clip, w_bl);
    out_clips.emplace_back(corner_points[1]->clip, w_br);
    out_clips.emplace_back(corner_points[2]->clip, w_tl);
    out_clips.emplace_back(corner_points[3]->clip, w_tr);
}

auto blend_space_def::get_parameter_count() const -> size_t
{
    return parameter_count_;
}

void animation_player::blend_to(const asset_handle<animation_clip>& clip,
                                seconds_t duration,
                                const blend_easing_t& easing)
{
    if(!clip)
    {
        if(current_layer_.state.clip)
        {
            current_layer_ = {};
        }
    }

    if(target_layer_.state.clip == clip)
    {
        return;
    }

    if(current_layer_.state.clip == clip)
    {
        return;
    }

    target_layer_.state.clip = clip;
    target_layer_.state.elapsed = seconds_t(0);

    // Set blending parameters
    blend_state_.state = blend_over_time{duration};
    blend_state_.easing = easing;
}

void animation_player::set_blend_space(const std::shared_ptr<blend_space_def>& blend_space)
{
    if(current_layer_.state.blend_space == blend_space)
    {
        return;
    }

    current_layer_.state.blend_space = blend_space;
    current_layer_.state.elapsed = seconds_t(0);

    // Clear target state if any
    target_layer_ = {};
    blend_state_ = {};
}

void animation_player::set_blend_space_parameters(const std::vector<float>& params)
{
    current_layer_.parameters = params;
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
    current_layer_.state.elapsed = seconds_t(0);
    target_layer_.state.elapsed = seconds_t(0);
}

void animation_player::update(seconds_t delta_time, const update_callback_t& set_transform_callback, bool force)
{
    if((!current_layer_.is_valid() && !target_layer_.is_valid()) || (!force && !is_playing()))
    {
        return;
    }

    // Update times
    if(playing_ && !paused_)
    {
        update_state(delta_time, current_layer_.state);

        update_state(delta_time, target_layer_.state);


        // update overtime parameters
        hpp::visit(hpp::overload(
                       [&](blend_over_time& state)
                       {
                           state.elapsed += delta_time;
                       },
                       [](auto& state)
                       {

                       }),
                   blend_state_.state);
    }

    // Update current layer
    update_pose(current_layer_);

    auto final_pose = &current_layer_.pose;

    // Update target layer
    if(update_pose(target_layer_))
    {
        // Compute blend factor
        float blend_progress = get_blend_progress();
        float blend_factor = compute_blend_factor(blend_progress);

        // Blend poses
        blend_poses(current_layer_.pose, target_layer_.pose, blend_factor, blend_pose_);
        final_pose = &blend_pose_;

        // Check if blending is finished
        if(blend_progress >= 1.0f)
        {
            // Switch to target animation or blend space
            current_layer_ = target_layer_;
            target_layer_ = {};
            blend_state_ = {};
        }
    }

    // Apply the final pose using the callback
    for(const auto& node : final_pose->nodes)
    {
        set_transform_callback(node.index, node.transform);
    }
}

auto animation_player::update_pose(animation_layer& layer) -> bool
{
    auto& state = layer.state;
    auto& pose = layer.pose;
    auto& parameters = layer.parameters;

    if(state.blend_space)
    {
        // Compute blending weights based on current parameters (e.g., speed and direction)
        state.blend_space->compute_blend(parameters, state.blend_clips);

        // Sample animations and blend poses
        state.blend_poses.resize(state.blend_clips.size());
        for(size_t i = 0; i < state.blend_clips.size(); ++i)
        {
            const auto& clip_weight_pair = state.blend_clips[i];
            sample_animation(clip_weight_pair.first.get().get(), state.elapsed, state.blend_poses[i]);
        }

        // Blend all poses based on their weights
        pose.nodes.clear();
        if(!state.blend_poses.empty())
        {
            // Initialize with the first pose
            pose = state.blend_poses[0];
            float total_weight = state.blend_clips[0].second;

            for(size_t i = 1; i < state.blend_poses.size(); ++i)
            {
                blend_poses(pose,
                            state.blend_poses[i],
                            state.blend_clips[i].second / (total_weight + state.blend_clips[i].second),
                            pose);
                total_weight += state.blend_clips[i].second;
            }
        }
        return true;
    }
    else if(state.clip)
    {
        sample_animation(state.clip.get().get(), state.elapsed, pose);
        return true;
    }

    return false;
}

void animation_player::update_state(seconds_t delta_time,animation_state& state)
{
    if(state.clip)
    {
        state.elapsed += delta_time;
        auto target_anim = state.clip.get();
        if(state.elapsed > target_anim->duration)
        {
            state.elapsed =
                seconds_t(std::fmod(state.elapsed.count(), target_anim->duration.count()));
        }
    }
}

auto animation_player::get_blend_progress() const -> float
{
    return hpp::visit(hpp::overload(
                          [](const hpp::monostate& state)
                          {
                              return 0.0f;
                          },
                          [](const auto& state)
                          {
                              return state.get_progress();
                          }),
                      blend_state_.state);
}

auto animation_player::compute_blend_factor(float normalized_blend_time) noexcept -> float
{
    float blend_factor = 0.0f;

    // Apply the easing function
    blend_factor = blend_state_.easing(normalized_blend_time);

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
