#pragma once
#include <engine/engine_export.h>
#include <engine/threading/threader.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <filesystem/filesystem.h>
#include <monort/monort.h>

#include "script_glue.h"

namespace ace
{

struct script_system
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    static void set_needs_recompile(const fs::path& protocol);
    static auto get_lib_name(const fs::path& protocol) -> fs::path;
    static auto get_lib_data_key(const fs::path& protocol) -> fs::path;
    static auto get_lib_compiled_key(const fs::path& protocol) -> fs::path;

    void load_core_domain(rtti::context& ctx);
    void unload_core_domain();

    void load_app_domain(rtti::context& ctx);
    void unload_app_domain();

private:
    /**
     * @brief Updates the physics system for each frame.
     * @param ctx The context for the update.
     * @param dt The delta time for the frame.
     */
    void on_frame_update(rtti::context& ctx, delta_t dt);

    /**
     * @brief Called when playback begins.
     * @param ctx The context for the playback.
     */
    void on_play_begin(rtti::context& ctx);

    /**
     * @brief Called when playback ends.
     * @param ctx The context for the playback.
     */
    void on_play_end(rtti::context& ctx);

    /**
     * @brief Called when playback is paused.
     * @param ctx The context for the playback.
     */
    void on_pause(rtti::context& ctx);

    /**
     * @brief Called when playback is resumed.
     * @param ctx The context for the playback.
     */
    void on_resume(rtti::context& ctx);

    /**
     * @brief Skips the next frame update.
     * @param ctx The context for the update.
     */
    void on_skip_next_frame(rtti::context& ctx);

    auto get_engine_assembly() const -> mono::mono_assembly;
    auto get_app_assembly() const -> mono::mono_assembly;

    void check_for_recompile(rtti::context& ctx, delta_t dt);

    auto create_compilation_job(rtti::context& ctx, const fs::path& protocol) -> itc::job_future<bool>;

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0); ///< Sentinel value to manage shared resources.

    delta_t time_since_last_check_{};
    script_glue glue_;

    std::unique_ptr<mono::mono_domain> domain_;
    std::unique_ptr<mono::mono_domain> app_domain_;
};
} // namespace ace
