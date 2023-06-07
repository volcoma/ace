#pragma once
#include <imgui/imgui.h>

#include <hpp/ring_buffer.hpp>
//#include <console/console.h>
#include <logging/logging.h>

#include <array>
#include <atomic>
#include <string>

namespace ace
{


class console_log : public sinks::base_sink<std::mutex>//, public console
{
public:
    struct log_entry
    {
        log_entry() = default;
        memory_buf_t formatted;
        level::level_enum level{level::off};
        source_loc source;
        // wrapping the formatted text with color (updated by pattern_formatter).
        mutable size_t color_range_start{0};
        mutable size_t color_range_end{0};
    };
	template <typename T>
	using ring_buffer = hpp::stack_ringbuffer<T, 150>;
	using entries_t = ring_buffer<log_entry>;

	void sink_it_(const details::log_msg& msg) override;
	void flush_() override;

    void draw();

    void draw_details();

private:

    void select_log(int index);
    void clear_log();
	bool has_new_entries() const;
	void set_has_new_entries(bool val);

    void draw_log(const log_entry& msg);
    void draw_range(const memory_buf_t& formatted, size_t start, size_t end);


	std::recursive_mutex entries_mutex_;
	///
	entries_t entries_;
	///
	std::atomic<bool> has_new_entries_ = {false};


    ImGuiTextFilter filter_;
    int selected_log_{-1};
};
}
