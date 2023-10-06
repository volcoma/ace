#pragma once
#include <imgui/imgui.h>

#include <hpp/optional.hpp>
#include <hpp/ring_buffer.hpp>
#include <hpp/small_vector.hpp>

// #include <console/console.h>
#include <logging/logging.h>

#include <array>
#include <atomic>
#include <string>

namespace ace
{

class console_log : public sinks::base_sink<std::mutex> //, public console
{
public:
    using mem_buf = hpp::small_vector<char, 250>;
    struct log_entry
    {
        mem_buf formatted;
        level::level_enum level{level::off};
        source_loc source;
        // wrapping the formatted text with color (updated by pattern_formatter).
        mutable size_t color_range_start{0};
        mutable size_t color_range_end{0};

        uint64_t id{};
    };

    using display_entries_t = hpp::small_vector<log_entry, 150>;
    using entries_t = hpp::stack_ringbuffer<log_entry, 150>;

    void sink_it_(const details::log_msg& msg) override;
    void flush_() override;

    void draw();

    void draw_details();

private:
    void select_log(const log_entry& entry);
    void clear_log();
    auto has_new_entries() const -> bool;
    void set_has_new_entries(bool val);

    void draw_log(const log_entry& msg);
    void draw_range(const mem_buf& formatted, size_t start, size_t end);

    std::recursive_mutex entries_mutex_;
    ///
    entries_t entries_;
    ///
    std::atomic<bool> has_new_entries_ = {false};

    ImGuiTextFilter filter_;

    uint64_t current_id_{};
    hpp::optional<log_entry> selected_log_{};
};
} // namespace ace
