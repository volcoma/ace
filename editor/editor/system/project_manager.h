#pragma once
#include <context/context.hpp>
#include <filesystem/syncer.h>

#include <deque>
#include <mutex>

namespace ace
{
class project_manager
{
public:
    struct options
    {
        ///
        std::deque<std::string> recent_project_paths;
    };

    project_manager(rtti::context& ctx);
    ~project_manager();

    bool open_project(rtti::context& ctx, const fs::path& project_path);


    void close_project(rtti::context& ctx);


    void create_project(rtti::context& ctx, const fs::path& project_path);


    void save_config();

    void load_config();

    inline const std::string& get_name() const
    {
        return project_name_;
    }

    inline void set_name(const std::string& name)
    {
        project_name_ = name;
    }

    inline options& get_options()
    {
        return options_;
    }

private:
    void setup_directory(rtti::context& ctx, fs::syncer& syncer);
    void setup_meta_syncer(rtti::context& ctx, fs::syncer& syncer, const fs::path& data_dir, const fs::path& meta_dir);
    void setup_cache_syncer(rtti::context& ctx, std::vector<uint64_t>& watchers,
                            fs::syncer& syncer,
                            const fs::path& meta_dir,
                            const fs::path& cache_dir);
    /// Project options
    options options_;
    /// Current project name
    std::string project_name_;

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
