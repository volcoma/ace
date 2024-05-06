#pragma once

#include "asset_handle.h"

#include <context/context.hpp>
#include <hpp/event.hpp>

#include <cassert>
#include <functional>
#include <unordered_map>

namespace ace
{

struct asset_meta
{
    hpp::uuid uid{};
    std::string type{};
};

class asset_database
{
public:
    struct meta
    {
        std::string location{};

        asset_meta meta;
    };
    using database_t = std::map<hpp::uuid, meta>;

    auto get_database() const -> const database_t&
    {
        return asset_meta_;
    }
    void set_database(const database_t& rhs)
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);

        asset_meta_ = rhs;
    }

    void remove_all()
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);

        asset_meta_.clear();
    }

    auto add_asset(const std::string& location, const asset_meta& meta) -> hpp::uuid
    {
        {
            const auto& uid = get_uuid(location);
            if(uid != hpp::uuid{})
            {
                return uid;
            }
        }

        std::lock_guard<std::mutex> lock(asset_mutex_);

        auto& metainfo = asset_meta_[meta.uid];
        metainfo.location = location;
        //metainfo.meta = meta;
        APPLOG_INFO("{}::{} - {} -> {}", hpp::type_name_str(*this), __func__, hpp::to_string(meta.uid), location);

        return meta.uid;
    }

    auto get_uuid(const std::string& location) -> const hpp::uuid&
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);

        for(const auto& kvp : asset_meta_)
        {
            const auto& uid = kvp.first;
            const auto& metainfo = kvp.second;
            if(metainfo.location == location)
            {
                return uid;
            }
        }

        static const hpp::uuid uid;
        return uid;
    }

    auto get_metadata(const hpp::uuid& id) -> const meta&
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);

        auto it = asset_meta_.find(id);
        if(it == asset_meta_.end())
        {
            static const meta empty;
            return empty;
        }

        return it->second;
    }

    void rename_asset(const std::string& key, const std::string& new_key)
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);
        for(auto& kvp : asset_meta_)
        {
            auto& uid = kvp.first;
            auto& metainfo = kvp.second;
            if(metainfo.location == key)
            {
                APPLOG_INFO("{}::{}::{} - {} -> {}",
                            hpp::type_name_str(*this),
                            __func__,
                            hpp::to_string(uid),
                            key,
                            new_key);

                metainfo.location = new_key;
            }
        }
    }

private:
    std::mutex asset_mutex_{};
    database_t asset_meta_{};
};

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

    using predicate_t = callable<bool(const asset_handle<T>&)>;

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
            if(predicate(it->second))
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
                                  const auto& id = it.id();

                                  hpp::string_view id_view(id);
                                  return id_view.starts_with(group);
                              });
    }

    void unload_single(itc::thread_pool& pool, const std::string& key) final
    {
        unload_with_condition(pool,
                              [&](const auto& it)
                              {
                                  const auto& id = it.id();

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
            if(predicate(kvp.second))
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
                const auto& id = it.id;

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
