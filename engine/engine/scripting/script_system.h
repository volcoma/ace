#pragma once
#include <engine/engine_export.h>

#include <engine/threading/threader.h>
#include <filesystem/filesystem.h>
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <monort/monort.h>
#include "script_glue.h"

namespace ace
{

struct script_system
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    static void set_needs_recompile(const fs::path& protocol);


    void load_core_domain(rtti::context& ctx);
    void unload_core_domain();

    void load_app_domain(rtti::context& ctx);
    void unload_app_domain();
private:

    /**
     * @brief Updates the script system for each frame.
     * @param ctx The context for the update.
     * @param dt The delta time for the frame.
     */
    void on_frame_update(rtti::context& ctx, delta_t dt);

    auto get_engine_assembly() const -> mono::mono_assembly;
    auto get_app_assembly() const -> mono::mono_assembly;

    auto create_compilation_job(rtti::context& ctx, const fs::path& protocol) -> itc::job_future<bool>;

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0); ///< Sentinel value to manage shared resources.


    delta_t time_since_last_check_{};
    script_glue glue_;

    std::unique_ptr<mono::mono_domain> domain_;
    std::unique_ptr<mono::mono_domain> app_domain_;


};
} // namespace ace
