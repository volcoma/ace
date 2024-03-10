#pragma once
#include "../../threading/threader.h"
#include "../asset_handle.h"

namespace ace::asset_reader
{

auto resolve_compiled_key(const std::string& key) -> std::string;
auto resolve_compiled_path(const std::string& key) -> fs::path;

template<typename T>
auto load_from_file(itc::thread_pool& pool, asset_handle<T>& output, const std::string& key) -> bool;

template<typename T>
inline auto load_from_instance(itc::thread_pool& pool, asset_handle<T>& output, std::shared_ptr<T> instance) -> bool
{
    auto job = pool.schedule(
                       [](std::shared_ptr<T> instance)
                       {
                           return instance;
                       },
                       instance)
                   .share();

    output.set_internal_job(job);

    return true;
}
} // namespace ace::asset_reader
