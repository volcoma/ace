#pragma once

#include "asset_handle.h"

#include <context/context.hpp>

#include <cassert>
#include <functional>
#include <unordered_map>

namespace ace
{

struct basic_storage
{
    virtual ~basic_storage() = default;

    virtual void unload_all(itc::thread_pool& pool) = 0;
    virtual void unload_single(itc::thread_pool& pool, const std::string& key) = 0;
    virtual void unload_group(itc::thread_pool& pool, const std::string& group) = 0;
};

template<typename T>
struct asset_storage : public basic_storage
{
    /// aliases
    using request_container_t = std::unordered_map<std::string, asset_handle<T>>;
    template<typename F>
    using callable = std::function<F>;
    using load_from_file_t = callable<bool(itc::thread_pool& pool, asset_handle<T>&, const std::string&)>;
    using load_from_instance_t = callable<bool(itc::thread_pool& pool, asset_handle<T>&, std::shared_ptr<T>)>;

    using predicate_t = callable<bool(const typename request_container_t::value_type&)>;

    ~asset_storage() override = default;

    void unload_handle(itc::thread_pool& pool, asset_handle<T>& handle)
    {
        pool.stop(handle.task_id());
        handle.invalidate();
    }

    void unload_with_condition(itc::thread_pool& pool, const predicate_t& predicate)
    {
        std::lock_guard<std::recursive_mutex> lock(container_mutex);
        for(auto it = container.begin(); it != container.end();)
        {
            if(predicate(*it))
            {
                auto& handle = it->second;
                unload_handle(pool, handle);
                it = container.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void unload_all(itc::thread_pool& pool) final
    {
        unload_with_condition(pool,
                              [&](const auto& it)
                              {
                                  return true;
                              });
    }

    void unload_group(itc::thread_pool& pool, const std::string& group) final
    {
        unload_with_condition(pool,
                              [&](const auto& it)
                              {
                                  const auto& id = it.first;

                                  hpp::string_view id_view(id);
                                  return id_view.starts_with(group);
                              });
    }

    void unload_single(itc::thread_pool& pool, const std::string& key) final
    {
        unload_with_condition(pool,
                              [&](const auto& it)
                              {
                                  const auto& id = it.first;
                                  const auto& handle = it.second;

                                  return id == key;
                              });
    }

    auto get_with_condition(const predicate_t& predicate) const -> std::vector<asset_handle<T>>
    {
        std::lock_guard<std::recursive_mutex> lock(container_mutex);
        std::vector<asset_handle<T>> result;

        result.reserve(container.size() + 1);
        result.emplace_back(asset_handle<T>::get_empty());

        for(const auto& kvp : container)
        {
            if(predicate(kvp))
            {
                result.emplace_back(kvp.second);
            }
        }

        return result;
    }

    auto get_group(const std::string& group) const -> std::vector<asset_handle<T>>
    {
        return get_with_condition(
            [&](const auto& it)
            {
                const auto& id = it.first;

                hpp::string_view id_view(id);
                return id_view.starts_with(group);
            });
    }

    /// key, mode
    load_from_file_t load_from_file;

    /// key, mode
    load_from_instance_t load_from_instance;

    /// Storage container
    request_container_t container;

    /// Mutex
    mutable std::recursive_mutex container_mutex;
};
} // namespace ace
