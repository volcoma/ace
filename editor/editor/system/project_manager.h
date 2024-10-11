#pragma once
#include <context/context.hpp>
#include <engine/settings/settings.h>
#include <editor/deploy/deploy.h>
#include <filesystem/syncer.h>
#include <deque>

namespace ace
{
class project_manager
{
public:
    struct project
    {
        std::string path;
    };


    struct options
    {
        ///
        std::vector<project> recent_projects;
    };

    project_manager();
    ~project_manager();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    auto open_project(rtti::context& ctx, const fs::path& project_path) -> bool;

    void close_project(rtti::context& ctx);

    void create_project(rtti::context& ctx, const fs::path& project_path);

    void save_config();

    void load_config();

    auto get_name() const -> const std::string&;

    void set_name(const std::string& name);

    auto get_settings() -> settings&;
    auto get_deploy_settings() -> deploy_settings&;

    auto get_options() -> options&;

    auto has_open_project() const -> bool;

    void load_project_settings();
    void save_project_settings();
    void load_deploy_settings();
    void save_deploy_settings();

private:
    void setup_directory(rtti::context& ctx, fs::syncer& syncer);
    void setup_meta_syncer(rtti::context& ctx, fs::syncer& syncer, const fs::path& data_dir, const fs::path& meta_dir);
    void setup_cache_syncer(rtti::context& ctx,
                            std::vector<uint64_t>& watchers,
                            fs::syncer& syncer,
                            const fs::path& meta_dir,
                            const fs::path& cache_dir);
    /// Project options
    options options_;


    /// Current project name
    std::string project_name_;
    settings project_settings_;
    deploy_settings deploy_settings_;

    fs::syncer app_meta_syncer_;
    fs::syncer app_cache_syncer_;
    std::vector<std::uint64_t> app_watchers_;

    fs::syncer editor_meta_syncer_;
    fs::syncer editor_cache_syncer_;
    std::vector<std::uint64_t> editor_watchers_;

    fs::syncer engine_meta_syncer_;
    fs::syncer engine_cache_syncer_;
    std::vector<std::uint64_t> engine_watchers_;
};
} // namespace ace
