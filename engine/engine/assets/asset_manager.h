#pragma once

#include <functional>
#include <unordered_map>

#include "asset_flags.h"
#include "asset_storage.h"
#include <cassert>

namespace ace
{

class asset_manager
{
public:
    asset_manager(rtti::context& ctx);
    ~asset_manager();

    auto init() -> bool;
    void unload_all();
    void unload_group(const std::string& group);

    template<typename S, typename... Args>
    auto add_storage(Args&&... args) -> asset_storage<S>&
    {
        auto operation = storages_.emplace(rtti::type_id<asset_storage<S>>().hash_code(),
                                           std::make_unique<asset_storage<S>>(std::forward<Args>(args)...));

        return static_cast<asset_storage<S>&>(*operation.first->second);
    }

    template<typename T>
    auto load(const std::string& key, load_flags flags = load_flags::standard) -> asset_handle<T>
    {
        auto& storage = get_storage<T>();
        return load_asset_from_file_impl<T>(key,
                                            flags,
                                            storage.container_mutex,
                                            storage.container,
                                            storage.load_from_file);
    }

    template<typename T>
    auto find_asset_entry(const std::string& key) const -> asset_handle<T>
    {
        auto& storage = get_storage<T>();
        return find_asset_impl<T>(key, storage.container_mutex, storage.container);
    }

    template<typename T>
    auto load_asset_from_instance(const std::string& key, std::shared_ptr<T> entry) -> asset_handle<T>
    {
        auto& storage = get_storage<T>();
        return load_asset_from_instance_impl(key,
                                             entry,
                                             storage.container_mutex,
                                             storage.container,
                                             storage.load_from_instance);
    }

    template<typename T>
    void rename_asset(const std::string& key, const std::string& new_key)
    {
        auto& storage = get_storage<T>();

        std::lock_guard<std::recursive_mutex> lock(storage.container_mutex);
        auto it = storage.container.find(key);
        if(it != storage.container.end())
        {
            auto& handle = it->second;
            handle.set_internal_id(new_key);
            storage.container[new_key] = handle;
            storage.container.erase(it);
        }
    }

    template<typename T>
    void unload_asset(const std::string& key)
    {
        auto& storage = get_storage<T>();
        storage.unload_single(pool_, key);
    }

    template<typename T>
    auto get_assets(const std::string& group = {}) const -> std::vector<asset_handle<T>>
    {
        auto& storage = get_storage<T>();
        return storage.get_group(group);
    }

    template<typename T, typename F>
    auto get_assets(F&& predicate) const -> std::vector<asset_handle<T>>
    {
        auto& storage = get_storage<T>();
        return storage.get_with_condition(predicate);
    }

    template<typename T, typename F>
    void for_each_asset(F&& callback)
    {
        auto& storage = get_storage<T>();
        std::lock_guard<std::recursive_mutex> lock(storage.container_mutex);

        for(const auto& el : storage.contaier)
        {
            callback(el);
        }
    }

private:
    template<typename T, typename F>
    auto load_asset_from_file_impl(const std::string& key,
                                   load_flags flags,
                                   std::recursive_mutex& container_mutex,
                                   typename asset_storage<T>::request_container_t& container,
                                   F&& load_func) -> asset_handle<T>
    {
        std::unique_lock<std::recursive_mutex> lock(container_mutex);

        if(flags != load_flags::reload)
        {
            auto it = container.find(key);
            if(it != std::end(container))
            {
                return it->second;
            }
        }

        auto& handle = container[key];
        // Dispatch the loading
        if(load_func)
        {
            // calling the function on a locked mutex is ok
            // since we dont expect this to actually
            // do much except add tasks to the executor

            if(handle.task_id())
            {
                pool_.stop(handle.task_id());
                handle.invalidate();
            }

            handle.set_internal_id(key);
            load_func(pool_, handle, key);
        }

        return handle;
    }

    template<typename T, typename F>
    auto load_asset_from_instance_impl(const std::string& key,
                                       std::shared_ptr<T> entry,
                                       std::recursive_mutex& container_mutex,
                                       typename asset_storage<T>::request_container_t& container,
                                       F&& load_func) -> asset_handle<T>
    {
        std::lock_guard<std::recursive_mutex> lock(container_mutex);
        auto it = container.find(key);
        if(it != std::end(container))
        {
            return it->second;
        }

        auto& handle = container[key];
        // Dispatch the loading
        if(load_func)
        {
            // loading on a locked mutex is ok
            // since we dont expect this to actually
            // do much except add tasks to the
            // executor
            handle.set_internal_id(key);
            load_func(pool_, handle, entry);
        }

        return handle;
    }

    template<typename T>
    auto find_asset_impl(const std::string& key,
                         std::recursive_mutex& container_mutex,
                         typename asset_storage<T>::request_container_t& container) const -> asset_handle<T>
    {
        std::lock_guard<std::recursive_mutex> lock(container_mutex);
        auto it = container.find(key);
        if(it != container.end())
        {
            return it->second;
        }

        return {};
    }

    template<typename S>
    auto get_storage() -> asset_storage<S>&
    {
        auto it = storages_.find(rtti::type_id<asset_storage<S>>().hash_code());
        assert(it != storages_.end());
        return (static_cast<asset_storage<S>&>(*it->second.get()));
    }

    template<typename S>
    auto get_storage() const -> asset_storage<S>&
    {
        auto& storage = storages_.at(rtti::type_id<asset_storage<S>>().hash_code());
        return (static_cast<asset_storage<S>&>(*storage.get()));
    }

    itc::thread_pool& pool_;
    /// Different storages
    std::unordered_map<std::size_t, std::unique_ptr<basic_storage>> storages_;
};
} // namespace ace
