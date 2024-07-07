#pragma once

#include <chrono>
#include <cstdint>

namespace ace
{

class performance_profiler
{
public:
    struct per_frame_data
    {
        float time = 0.0f;
        uint32_t samples = 0;

        per_frame_data() = default;
        per_frame_data(float t) : time(t)
        {
        }

        operator float() const
        {
            return time;
        }
        inline per_frame_data& operator+=(float t)
        {
            time += t;
            return *this;
        }
    };

    using record_data_t = std::unordered_map<const char*, per_frame_data>;

    void add_record(const char* name, float time)
    {
        auto& per_frame = get_per_frame_data_write();
        if(per_frame.find(name) == per_frame.end())
            per_frame[name] = 0.0f;

        auto& data = per_frame[name];
        data.time += time;
        data.samples++;
    }

    void swap()
    {
        current_ = get_next_index();
        get_per_frame_data_write().clear();
    }

    auto get_per_frame_data_read() const -> const record_data_t&
    {
        return per_frame_data_[get_next_index()];
    }

    auto get_per_frame_data_write() -> record_data_t&
    {
        return per_frame_data_[current_];
    }

private:
    auto get_next_index() const -> int
    {
        return (current_ + 1) / per_frame_data_.size();
    }

    std::array<record_data_t, 2> per_frame_data_;
    int current_{0};
};

class scope_perf_timer
{
public:
    using clock_t = std::chrono::high_resolution_clock;
    using timepoint_t = clock_t::time_point;
    using duration_t = std::chrono::duration<float, std::milli>;

    scope_perf_timer(const char* name, performance_profiler* profiler) : name_(name), profiler_(profiler)
    {
    }

    ~scope_perf_timer()
    {
        auto end = clock_t::now();
        auto time = std::chrono::duration_cast<duration_t>(end - start_);

        profiler_->add_record(name_, time.count());
    }

private:
    const char* name_;
    performance_profiler* profiler_{};
    timepoint_t start_ = clock_t::now();
};

auto get_app_profiler() -> performance_profiler*;

#define APP_SCOPE_PERF(name) scope_perf_timer timer__LINE__(name, get_app_profiler())

} // namespace ace
