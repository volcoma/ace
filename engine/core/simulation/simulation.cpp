#include "simulation.h"
#include <algorithm>
#include <thread>
#include <limits>

namespace ace
{
using namespace std::chrono_literals;

simulation::simulation()
{
    if(max_inactive_fps_ == 0)
    {
        max_inactive_fps_ = std::max(max_inactive_fps_, max_fps_);
    }
}

void simulation::run_one_frame(bool is_active)
{
    // perform waiting loop if maximum fps set
    auto max_fps = max_fps_;
    if(!is_active && max_fps > 0)
    {
        max_fps = std::min(max_inactive_fps_, max_fps);
    }

    duration_t elapsed = clock_t::now() - last_frame_timepoint_;
    if(max_fps > 0)
    {
        duration_t target_duration = 1000ms / max_fps;

        for(;;)
        {
            elapsed = clock_t::now() - last_frame_timepoint_;
            if(elapsed >= target_duration)
            {
                break;
            }

            if(elapsed < duration_t(0))
            {
                break;
            }
            duration_t sleep_time = (target_duration - elapsed);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(sleep_time);

            if(sleep_time > std::chrono::microseconds(1000))
            {
                if(ms.count() > 0)
                {
                    sleep_time /= ms.count();
                }

                std::this_thread::sleep_for(sleep_time);
            }
        }
    }

    if(elapsed < duration_t(0))
    {
        elapsed = duration_t(0);
    }
    last_frame_timepoint_ = clock_t::now();

    // if fps lower than minimum, clamp eplased time
    if(min_fps_ > 0)
    {
        duration_t target_duration = 1000ms / min_fps_;
        if(elapsed > target_duration)
        {
            elapsed = target_duration;
        }
    }

    // perform time step smoothing
    if(smoothing_step_ > 0)
    {
        timestep_ = duration_t::zero();
        previous_timesteps_.push_back(elapsed);
        if(previous_timesteps_.size() > smoothing_step_)
        {
            auto begin = previous_timesteps_.begin();
            previous_timesteps_.erase(begin, begin + int(previous_timesteps_.size() - smoothing_step_));
            for(auto step : previous_timesteps_)
            {
                timestep_ += step;
            }
            timestep_ /= static_cast<duration_t::rep>(previous_timesteps_.size());
        }
        else
        {
            timestep_ = previous_timesteps_.back();
        }
    }
    else
    {
        timestep_ = elapsed;
    }

    ++frame_;
}

auto simulation::get_frame() const -> uint64_t
{
    return frame_;
}

void simulation::set_min_fps(uint32_t fps)
{
    min_fps_ = std::max<uint32_t>(fps, 0);
}

void simulation::set_max_fps(uint32_t fps)
{
    max_fps_ = std::max<uint32_t>(fps, 0);
}

void simulation::set_max_inactive_fps(uint32_t fps)
{
    max_inactive_fps_ = std::max<uint32_t>(fps, 0);
}

void simulation::set_time_smoothing_step(uint32_t step)
{
    smoothing_step_ = step;
}

auto simulation::get_time_since_launch() const -> duration_t
{
    return clock_t::now() - launch_timepoint_;
}

auto simulation::get_fps() const -> float
{
    auto dt = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(get_delta_time()).count();
    return (std::abs(dt) < std::numeric_limits<float>::epsilon()) ? 0 : 1000.0f / dt;
}

auto simulation::get_delta_time() const -> delta_t
{
    auto dt = std::chrono::duration_cast<delta_t>(timestep_) * time_scale_;
    return dt;
}

void simulation::set_time_scale(float time_scale)
{
    time_scale_ = time_scale;
}

auto simulation::get_time_scale() const -> float
{
    return time_scale_;
}
} // namespace ace
