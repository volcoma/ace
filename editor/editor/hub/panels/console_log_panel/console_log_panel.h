#pragma once
#include <editor/imgui/integration/imgui.h>

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

class console_log_panel : public sinks::base_sink<std::mutex> //, public console
{
public:
    using mem_buf = hpp::small_vector<char, 250>;
    struct log_entry
    {
        mem_buf formatted;

        level::level_enum level{level::off};
        source_loc source;

        uint64_t id{};
    };

    using display_entries_t = hpp::small_vector<log_entry, 150>;
    using entries_t = hpp::stack_ringbuffer<log_entry, 150>;

    console_log_panel();
    void sink_it_(const details::log_msg& msg) override;
    void flush_() override;

    void on_frame_ui_render(rtti::context& ctx, const char* name);
    void draw();

    void draw_details();
    auto draw_last_log() -> bool;

    void draw_last_log_button();

private:
    void select_log(const log_entry& entry);
    void clear_log();
    void open_log(const log_entry& entry);
    auto has_new_entries() const -> bool;
    void set_has_new_entries(bool val);

    auto draw_log(const log_entry& msg, int num_lines) -> bool;
    void draw_range(const hpp::string_view& formatted, size_t start, size_t end);

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
