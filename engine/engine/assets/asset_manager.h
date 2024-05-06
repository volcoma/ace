#pragma once

#include "asset_flags.h"
#include "asset_storage.h"
#include <cassert>
#include <unordered_map>

namespace ace
{

class asset_manager
{
public:
    asset_manager(rtti::context& ctx);
    ~asset_manager();

    void set_parent(asset_manager* parent);

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void unload_all();
    void unload_group(const std::string& group);

    auto load_database(const std::string& protocol) -> bool;
    void save_database(const std::string& protocol, const fs::path& path);


    auto add_asset_info_for_path(const fs::path& path, const asset_meta& meta) -> hpp::uuid;
    auto add_asset_info_for_key(const std::string& key, const asset_meta& meta) -> hpp::uuid;

    template<typename S, typename... Args>
    auto add_storage(Args&&... args) -> asset_storage<S>&
    {
        auto operation = storages_.emplace(hpp::type_id<asset_storage<S>>().hash_code(),
                                           std::make_unique<asset_storage<S>>(std::forward<Args>(args)...));

        return static_cast<asset_storage<S>&>(*operation.first->second);
    }

    template<typename T>
    auto get_asset(const std::string& key, load_flags flags = load_flags::standard) -> asset_handle<T>
    {
        auto& storage = get_storage<T>();
        return load_asset_from_file_impl<T>(key,
                                            flags,
                                            storage.container_mutex,
                                            storage.container,
                                            storage.load_from_file);
    }

    template<typename T>
    auto get_asset(const hpp::uuid& uid, load_flags flags = load_flags::standard) -> asset_handle<T>
    {
        for(auto& kvp : databases_)
        {
            auto& db = kvp.second;
            const auto& meta = db.get_metadata(uid);
            if(!meta.location.empty())
            {
                const auto& key = meta.location;

                return get_asset<T>(key, flags);
            }
        }

        if(parent_)
        {
            return parent_->get_asset<T>(uid, flags);
        }
        return {};
    }

    template<typename T>
    auto find_asset(const std::string& key) const -> const asset_handle<T>&
    {
        auto& storage = get_storage<T>();
        return find_asset_impl<T>(key, storage.container_mutex, storage.container);
    }

    template<typename T>
    auto get_asset_from_instance(const std::string& key, std::shared_ptr<T> entry) -> asset_handle<T>
    {
        auto& storage = get_storage<T>();
        return get_asset_from_instance_impl(key,
                                            entry,
                                            storage.container_mutex,
                                            storage.container,
                                            storage.load_from_instance);
    }

    template<typename T>
    void rename_asset(const std::string& key, const std::string& new_key)
    {
        {
            for(auto& kvp : databases_)
            {
                auto& db = kvp.second;
                db.rename_asset(key, new_key);
            }

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
        if(parent_)
        {
            parent_->rename_asset<T>(key, new_key);
        }
    }

    template<typename T>
    void unload_asset(const std::string& key)
    {
        auto& storage = get_storage<T>();
        storage.unload_single(pool_, key);

        if(parent_)
        {
            parent_->unload_asset<T>(key);
        }
    }

    template<typename T>
    auto get_assets(const std::string& group = {}) const -> std::vector<asset_handle<T>>
    {
        auto& storage = get_storage<T>();
        auto assets = storage.get_group(group);

        if(parent_)
        {
            auto passets = parent_->get_assets<T>(group);
            std::move(std::begin(passets), std::end(passets), std::back_inserter(assets));
        }

        return assets;
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
        {
            auto& storage = get_storage<T>();
            std::lock_guard<std::recursive_mutex> lock(storage.container_mutex);

            for(const auto& el : storage.contaier)
            {
                callback(el);
            }
        }

        if(parent_)
        {
            parent_->for_each_asset<T, F>(callback);
        }
    }

private:
    auto get_database(const std::string& group, bool exact_match = false) -> asset_database&;
    void remove_database(const std::string& group, bool exact_match = false);
    auto add_asset(const std::string& key) -> hpp::uuid;

    template<typename T, typename F>
    auto load_asset_from_file_impl(const std::string& key,
                                   load_flags flags,
                                   std::recursive_mutex& container_mutex,
                                   typename asset_storage<T>::request_container_t& container,
                                   F&& load_func) -> asset_handle<T>
    {
        if(flags != load_flags::reload)
        {
            auto inst = find_asset_impl<T>(key, container_mutex, container);
            if(inst)
            {
                return inst;
            }
        }

        std::unique_lock<std::recursive_mutex> lock(container_mutex);

        auto& handle = container[key];
        // Dispatch the loading
        if(load_func)
        {
            auto uid = add_asset(key);
            // calling the function on a locked mutex is ok
            // since we dont expect this to actually
            // do much except add tasks to the executor

            if(handle.task_id())
            {
                pool_.stop(handle.task_id());
                handle.invalidate();
            }

            handle.set_internal_ids(uid, key);
            load_func(pool_, handle, key);
        }

        return handle;
    }

    template<typename T, typename F>
    auto get_asset_from_instance_impl(const std::string& key,
                                      std::shared_ptr<T> entry,
                                      std::recursive_mutex& container_mutex,
                                      typename asset_storage<T>::request_container_t& container,
                                      F&& load_func) -> asset_handle<T>
    {
        auto inst = find_asset_impl<T>(key, container_mutex, container);
        if(inst)
        {
            return inst;
        }

        std::lock_guard<std::recursive_mutex> lock(container_mutex);

        auto& handle = container[key];
        // Dispatch the loading
        if(load_func)
        {
            auto uid = add_asset(key);

            // loading on a locked mutex is ok
            // since we dont expect this to actually
            // do much except add tasks to the
            // executor
            handle.set_internal_ids(uid, key);
            load_func(pool_, handle, entry);
        }

        return handle;
    }

    template<typename T>
    auto find_asset_impl(const std::string& key,
                         std::recursive_mutex& container_mutex,
                         typename asset_storage<T>::request_container_t& container) const -> const asset_handle<T>&
    {
        std::lock_guard<std::recursive_mutex> lock(container_mutex);
        auto it = container.find(key);
        if(it != container.end())
        {
            return it->second;
        }

        if(parent_)
        {
            return parent_->find_asset_impl<T>(key, container_mutex, container);
        }

        return asset_handle<T>::get_empty();
    }

    template<typename S>
    auto get_storage() -> asset_storage<S>&
    {
        auto it = storages_.find(hpp::type_id<asset_storage<S>>().hash_code());
        assert(it != storages_.end());
        return (static_cast<asset_storage<S>&>(*it->second.get()));
    }

    template<typename S>
    auto get_storage() const -> asset_storage<S>&
    {
        auto& storage = storages_.at(hpp::type_id<asset_storage<S>>().hash_code());
        return (static_cast<asset_storage<S>&>(*storage.get()));
    }

    itc::thread_pool& pool_;
    /// Different storages
    std::unordered_map<std::size_t, std::unique_ptr<basic_storage>> storages_{};

    std::mutex db_mutex_;
    std::map<std::string, asset_database, std::less<>> databases_{};

    asset_manager* parent_{};
};
} // namespace ace
