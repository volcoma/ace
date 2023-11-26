#pragma once
#include <context/context.hpp>
#include <filesystem/syncer.h>

#include <deque>
#include <mutex>

namespace ace
{
class asset_watcher
{
public:

    asset_watcher();
    ~asset_watcher();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void watch_assets(rtti::context& ctx, const std::string& protocol);
    void unwatch_assets(rtti::context& ctx, const std::string& protocol);

private:
    void setup_directory(rtti::context& ctx, fs::syncer& syncer);
    void setup_meta_syncer(rtti::context& ctx, fs::syncer& syncer, const fs::path& data_dir, const fs::path& meta_dir);
    void setup_cache_syncer(rtti::context& ctx,
                            std::vector<uint64_t>& watchers,
                            fs::syncer& syncer,
                            const fs::path& meta_dir,
                            const fs::path& cache_dir);

    struct watched
    {
        fs::syncer meta_syncer;
        fs::syncer cache_syncer;
        std::vector<std::uint64_t> watchers;
    };

    std::map<std::string, watched> watched_protocols_{};
};
} // namespace ace
