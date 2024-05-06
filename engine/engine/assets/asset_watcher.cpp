#include "asset_watcher.h"
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_compiler.h>
#include <engine/assets/impl/asset_extensions.h>

#include <engine/animation/animation.h>
#include <engine/audio/audio_clip.h>
#include <engine/ecs/ecs.h>
#include <engine/ecs/prefab.h>
#include <engine/physics/physics_material.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>

#include <engine/threading/threader.h>

#include <engine/meta/assets/asset_database.hpp>

#include <filesystem/watcher.h>
#include <graphics/graphics.h>
#include <logging/logging.h>

#include <fstream>

namespace ace
{
namespace
{
using namespace std::literals;

auto remove_meta_tag(const fs::path& synced_path) -> fs::path
{
    return fs::replace(synced_path, ".meta", "");
}

auto remove_meta_tag(const std::vector<fs::path>& synced_paths) -> std::vector<fs::path>
{
    std::decay_t<decltype(synced_paths)> reduced;
    reduced.reserve(synced_paths.size());
    for(const auto& synced_path : synced_paths)
    {
        reduced.emplace_back(remove_meta_tag(synced_path));
    }
    return reduced;
}

void unwatch(std::vector<uint64_t>& watchers)
{
    for(const auto& id : watchers)
    {
        fs::watcher::unwatch(id);
    }
    watchers.clear();
}

auto get_asset_key(const fs::path& path) -> std::string
{
    auto p = fs::reduce_trailing_extensions(path);
    auto data_key = fs::convert_to_protocol(p);
    auto key = fs::replace(data_key.generic_string(), ":/compiled", ":/data").generic_string();
    return key;
}

auto get_meta_key(const fs::path& path) -> std::string
{
    auto p = fs::reduce_trailing_extensions(path);
    auto data_key = fs::convert_to_protocol(p);
    auto key = fs::replace(data_key.generic_string(), ":/compiled", ":/meta").generic_string();
    return key + ".meta";
}

template<typename T>
auto watch_assets(rtti::context& ctx, const fs::path& dir, const fs::path& wildcard, bool reload_async) -> uint64_t
{
    auto& am = ctx.get<asset_manager>();
    auto& ts = ctx.get<threader>();

    fs::path watch_dir = (dir / wildcard).make_preferred();

    auto callback = [&am, &ts](const auto& entries, bool is_initial_list)
    {
        for(const auto& entry : entries)
        {
            auto key = get_asset_key(entry.path);

            APPLOG_TRACE("{}", fs::to_string(entry));

            if(entry.type == fs::file_type::regular)
            {
                if(entry.status == fs::watcher::entry_status::removed)
                {
                    am.unload_asset<T>(key);
                }
                else if(entry.status == fs::watcher::entry_status::renamed)
                {
                    auto old_key = get_asset_key(entry.last_path);
                    am.rename_asset<T>(old_key, key);
                }
                else // created or modified
                {
                    fs::error_code ec;
                    auto key_path = fs::resolve_protocol(key);
                    if(!fs::exists(key_path, ec))
                    {
                        APPLOG_ERROR("{} does not exist. Cleaning up cached...", key);
                        fs::remove(entry.path, ec);

                        auto meta = get_meta_key(entry.path);
                        auto meta_path = fs::resolve_protocol(meta);
                        if(fs::exists(meta_path, ec))
                        {
                            APPLOG_ERROR("{} does not exist. Cleaning up meta...", key);
                            fs::remove(meta_path, ec);
                        }
                    }
                    else
                    {
                        load_flags flags = is_initial_list ? load_flags::standard : load_flags::reload;
                        am.get_asset<T>(key, flags);
                    }
                }
            }
        }
    };

    return fs::watcher::watch(watch_dir, true, true, 500ms, callback);
}

template<typename T>
static void add_to_syncer(rtti::context& ctx,
                          std::vector<uint64_t>& watchers,
                          fs::syncer& syncer,
                          const fs::path& dir,
                          const fs::syncer::on_entry_removed_t& on_removed,
                          const fs::syncer::on_entry_renamed_t& on_renamed)
{
    auto& ts = ctx.get<threader>();
    auto& am = ctx.get<asset_manager>();

    auto on_modified =
        [&ts, &am](const std::string& ext, const auto& ref_path, const auto& synced_paths, bool is_initial_listing)
    {
        auto paths = remove_meta_tag(synced_paths);
        // if(paths.empty())
        // {
        //     return;
        //}
        // const auto& output = paths.front();
        for(const auto& output : paths)
        {
            fs::error_code err;
            if(is_initial_listing && fs::exists(output, err))
            {
                continue;
            }
            auto task = ts.pool->schedule(
                [&am, ref_path, output]()
                {
                    asset_compiler::compile<T>(am, ref_path, output);
                });

            //        if(is_initial_listing)
            //        {
            //            task.wait();
            //        }
        }
    };

    for(const auto& type : ex::get_suported_formats<T>())
    {
        syncer.set_mapping(type + ".meta", {".asset"}, on_modified, on_modified, on_removed, on_renamed);
        const auto watch_id = watch_assets<T>(ctx, dir, "*" + type, true);
        watchers.push_back(watch_id);
    }
}

template<>
void add_to_syncer<gfx::shader>(rtti::context& ctx,
                                std::vector<uint64_t>& watchers,
                                fs::syncer& syncer,
                                const fs::path& dir,
                                const fs::syncer::on_entry_removed_t& on_removed,
                                const fs::syncer::on_entry_renamed_t& on_renamed)
{
    auto& ts = ctx.get<threader>();
    auto& am = ctx.get<asset_manager>();

    auto on_modified =
        [&ts, &am](const std::string& ext, const auto& ref_path, const auto& synced_paths, bool is_initial_listing)
    {
        auto paths = remove_meta_tag(synced_paths);
        if(paths.empty())
        {
            return;
        }
        const auto& renderer_extension = gfx::get_renderer_filename_extension();
        auto it = std::find_if(std::begin(paths),
                               std::end(paths),
                               [&renderer_extension](const auto& key)
                               {
                                   return key.extension() == renderer_extension;
                               });

        if(it == std::end(paths))
        {
            return;
        }

        fs::path output = *it;

        // for(const auto& output : paths)
        {
            fs::error_code err;
            if(is_initial_listing && fs::exists(output, err))
            {
                return;
            }

            auto task = ts.pool->schedule(
                [&am, ref_path, output]()
                {
                    asset_compiler::compile<gfx::shader>(am, ref_path, output);
                });
            //        if(is_initial_listing)
            //        {
            //            task.wait();
            //        }
        }
    };

    const auto& renderer_extension = gfx::get_renderer_filename_extension();
    for(const auto& type : ex::get_suported_formats<gfx::shader>())
    {
        syncer.set_mapping(type + ".meta",
                           {".asset.dx11", ".asset.dx12", ".asset.gl", ".asset.spirv"},
                           on_modified,
                           on_modified,
                           on_removed,
                           on_renamed);

        const auto watch_id = watch_assets<gfx::shader>(ctx, dir, "*" + type + ".asset" + renderer_extension, true);
        watchers.push_back(watch_id);
    }
}

} // namespace

void asset_watcher::setup_directory(rtti::context& ctx, fs::syncer& syncer)
{
    const auto on_dir_modified =
        [](const std::string& ext, const auto& /*ref_path*/, const auto& /*synced_paths*/, bool /*is_initial_listing*/)
    {

    };
    const auto on_dir_removed = [](const std::string& ext, const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::remove_all(synced_path, err);
        }
    };

    const auto on_dir_renamed = [](const std::string& ext, const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::rename(synced_path.first, synced_path.second, err);
        }
    };
    syncer.set_directory_mapping(on_dir_modified, on_dir_modified, on_dir_removed, on_dir_renamed);
}

void asset_watcher::setup_meta_syncer(rtti::context& ctx,
                                      fs::syncer& syncer,
                                      const fs::path& data_dir,
                                      const fs::path& meta_dir,
                                      bool wait)
{
    setup_directory(ctx, syncer);
    auto& am = ctx.get<asset_manager>();

    const auto on_file_removed = [](const std::string& ext, const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::remove_all(synced_path, err);
        }
    };

    const auto on_file_renamed = [](const std::string& ext, const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            fs::error_code err;
            fs::rename(synced_path.first, synced_path.second, err);
        }
    };

    const auto on_file_modified =
        [&am](const std::string& ext, const auto& ref_path, const auto& synced_paths, bool is_initial_listing)
    {
        for(const auto& synced_path : synced_paths)
        {
            asset_meta meta;
            fs::error_code err;
            if(fs::exists(synced_path, err))
            {
                load_from_file(synced_path.string(), meta);
            }

            if(meta.uid.is_nil())
            {
                meta.uid = generate_uuid();
                meta.type = ext;
            }
            am.add_asset_info_for_path(ref_path, meta);

            save_to_file(synced_path.string(), meta);
        }
    };

    for(const auto& asset_set : ex::get_all_formats())
    {
        for(const auto& type : asset_set)
        {
            syncer.set_mapping(type, {".meta"}, on_file_modified, on_file_modified, on_file_removed, on_file_renamed);
        }
    }

    syncer.sync(data_dir, meta_dir);

    if(wait)
    {
        auto& ts = ctx.get<threader>();
        ts.pool->wait_all();
    }
}

void asset_watcher::setup_cache_syncer(rtti::context& ctx,
                                       std::vector<uint64_t>& watchers,
                                       fs::syncer& syncer,
                                       const fs::path& meta_dir,
                                       const fs::path& cache_dir,
                                       bool wait)
{
    setup_directory(ctx, syncer);

    auto on_removed = [](const std::string& ext, const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            auto synced_asset = remove_meta_tag(synced_path);
            fs::error_code err;
            fs::remove_all(synced_asset, err);
        }
    };

    auto on_renamed = [](const std::string& ext, const auto& /*ref_path*/, const auto& synced_paths)
    {
        for(const auto& synced_path : synced_paths)
        {
            auto synced_old_asset = remove_meta_tag(synced_path.first);
            auto synced_new_asset = remove_meta_tag(synced_path.second);
            fs::error_code err;
            fs::rename(synced_old_asset, synced_new_asset, err);
        }
    };

    add_to_syncer<gfx::texture>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<gfx::shader>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<mesh>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<material>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<animation>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<prefab>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<scene_prefab>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<physics_material>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);
    add_to_syncer<audio_clip>(ctx, watchers, syncer, cache_dir, on_removed, on_renamed);

    syncer.sync(meta_dir, cache_dir);

    if(wait)
    {
        auto& ts = ctx.get<threader>();
        ts.pool->wait_all();
    }
}

asset_watcher::asset_watcher()
{
}

asset_watcher::~asset_watcher()
{
}

auto asset_watcher::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    watch_assets(ctx, "engine:/", true);

    return true;
}

auto asset_watcher::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    unwatch_assets(ctx, "engine:/");
    return true;
}

void asset_watcher::watch_assets(rtti::context& ctx, const std::string& protocol, bool wait)
{
    auto& w = watched_protocols_[protocol];

    auto data_protocol = protocol + "data";
    auto meta_protocol = protocol + "meta";
    auto cache_protocol = protocol + "compiled";

    setup_meta_syncer(ctx,
                      w.meta_syncer,
                      fs::resolve_protocol(data_protocol),
                      fs::resolve_protocol(meta_protocol),
                      wait);

    setup_cache_syncer(ctx,
                       w.watchers,
                       w.cache_syncer,
                       fs::resolve_protocol(meta_protocol),
                       fs::resolve_protocol(cache_protocol),
                       wait);
}

void asset_watcher::unwatch_assets(rtti::context& ctx, const std::string& protocol)
{
    auto& w = watched_protocols_[protocol];

    unwatch(w.watchers);
    w.meta_syncer.unsync();
    w.cache_syncer.unsync();

    watched_protocols_.erase(protocol);

    auto& am = ctx.get<asset_manager>();
    am.unload_group(protocol);
}

} // namespace ace
