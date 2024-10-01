#pragma once

#include "asset_handle.h"

#include <context/context.hpp>
#include <hpp/event.hpp>

#include <cassert>
#include <functional>
#include <map>
#include <mutex>
#include <unordered_map>

namespace ace
{

/**
 * @struct asset_meta
 * @brief Metadata for an asset, including its UUID and type.
 */
struct asset_meta
{
    /// Unique identifier for the asset.
    hpp::uuid uid{};
    /// Type of the asset.
    std::string type{};
};

/**
 * @class asset_database
 * @brief Manages asset metadata and provides functionality for adding, removing, and querying assets.
 */
class asset_database
{
public:
    /**
     * @struct meta
     * @brief Metadata information for an asset including its location.
     */
    struct meta
    {
        /// Location of the asset.
        std::string location{};
        /// Metadata of the asset.
        asset_meta meta;
    };

    /// Type definition for the asset database.
    using database_t = std::map<hpp::uuid, meta>;

    /**
     * @brief Generates a UUID based on the file path.
     * @param p The file path.
     * @return The generated UUID.
     */
    static auto generate_id(const fs::path& p) -> hpp::uuid
    {
        return generate_uuid(p.generic_string());
    }

    /**
     * @brief Gets the entire asset database.
     * @return A constant reference to the asset database.
     */
    auto get_database() const -> const database_t&
    {
        return asset_meta_;
    }

    /**
     * @brief Sets the asset database.
     * @param rhs The asset database to set.
     */
    void set_database(const database_t& rhs)
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);
        asset_meta_ = rhs;
    }

    /**
     * @brief Removes all assets from the database.
     */
    void remove_all()
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);
        asset_meta_.clear();
    }

    /**
     * @brief Adds an asset to the database.
     * @param location The location of the asset.
     * @param meta The metadata of the asset.
     * @return The UUID of the added asset.
     */
    auto add_asset(const std::string& location, const asset_meta& meta) -> hpp::uuid
    {
        const auto& uid = get_uuid(location);
        if(uid != hpp::uuid{})
        {
            return uid;
        }

        std::lock_guard<std::mutex> lock(asset_mutex_);

        auto& metainfo = asset_meta_[meta.uid];
        metainfo.location = location;
        APPLOG_TRACE("{} - {} -> {}", __func__, hpp::to_string(meta.uid), location);

        return meta.uid;
    }

    /**
     * @brief Gets the UUID of an asset based on its location.
     * @param location The location of the asset.
     * @return The UUID of the asset.
     */
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

    /**
     * @brief Gets the metadata of an asset based on its UUID.
     * @param id The UUID of the asset.
     * @return The metadata of the asset.
     */
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

    /**
     * @brief Renames an asset.
     * @param key The current key of the asset.
     * @param new_key The new key for the asset.
     */
    void rename_asset(const std::string& key, const std::string& new_key)
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);
        for(auto& kvp : asset_meta_)
        {
            auto& uid = kvp.first;
            auto& metainfo = kvp.second;
            if(metainfo.location == key)
            {
                APPLOG_INFO("{}::{} - {} -> {}", __func__, hpp::to_string(uid), key, new_key);

                metainfo.location = new_key;
            }
        }
    }

    /**
     * @brief Removes an asset from the database.
     * @param key The key of the asset to remove.
     */
    void remove_asset(const std::string& key)
    {
        std::lock_guard<std::mutex> lock(asset_mutex_);
        for(auto& kvp : asset_meta_)
        {
            auto& uid = kvp.first;
            auto& metainfo = kvp.second;
            if(metainfo.location == key)
            {
                APPLOG_INFO("{}::{} - {}", __func__, hpp::to_string(uid), key);

                asset_meta_.erase(uid);
                return;
            }
        }
    }

private:
    /// Mutex for asset database operations.
    std::mutex asset_mutex_{};
    /// The asset database.+
    database_t asset_meta_{};
};

/**
 * @struct basic_storage
 * @brief Abstract base class for asset storage.
 */
struct basic_storage
{
    virtual ~basic_storage() = default;

    /**
     * @brief Unloads all assets.
     * @param pool The thread pool for unloading tasks.
     */
    virtual void unload_all(itc::thread_pool& pool) = 0;

    /**
     * @brief Unloads a single asset by its key.
     * @param pool The thread pool for unloading tasks.
     * @param key The key of the asset to unload.
     */
    virtual void unload_single(itc::thread_pool& pool, const std::string& key) = 0;

    /**
     * @brief Unloads all assets in a specified group.
     * @param pool The thread pool for unloading tasks.
     * @param group The group to unload.
     */
    virtual void unload_group(itc::thread_pool& pool, const std::string& group) = 0;
};

/**
 * @struct asset_storage
 * @brief Manages storage and loading of assets of a specific type.
 * @tparam T The type of the assets.
 */
template<typename T>
struct asset_storage : public basic_storage
{
    /// Container for asset requests.
    using request_container_t = std::unordered_map<std::string, asset_handle<T>>;
    /// Type alias for callable functions.
    template<typename F>
    using callable = std::function<F>;

    /// Function type for loading from file.
    using load_from_file_t = callable<bool(itc::thread_pool& pool, asset_handle<T>&, const std::string&)>;

    /// Function type for loading from instance. Predicate function type.
    using predicate_t = callable<bool(const asset_handle<T>&)>;
    using load_from_instance_t = callable<bool(itc::thread_pool& pool, asset_handle<T>&, std::shared_ptr<T>)>;

    ~asset_storage() override = default;

    /**
     * @brief Unloads a handle.
     * @param pool The thread pool for unloading tasks.
     * @param handle The handle to unload.
     */
    void unload_handle(itc::thread_pool& pool, asset_handle<T>& handle)
    {
        pool.stop(handle.task_id());
        handle.invalidate();
    }

    /**
     * @brief Unloads assets that satisfy a condition.
     * @param pool The thread pool for unloading tasks.
     * @param predicate The predicate function to determine which assets to unload.
     */
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

    /**
     * @brief Unloads all assets.
     * @param pool The thread pool for unloading tasks.
     */
    void unload_all(itc::thread_pool& pool) final
    {
        unload_with_condition(pool,
                              [](const auto& it)
                              {
                                  return true;
                              });
    }

    /**
     * @brief Unloads all assets in a specified group.
     * @param pool The thread pool for unloading tasks.
     * @param group The group to unload.
     */
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

    /**
     * @brief Unloads a single asset by its key.
     * @param pool The thread pool for unloading tasks.
     * @param key The key of the asset to unload.
     */
    void unload_single(itc::thread_pool& pool, const std::string& key) final
    {
        unload_with_condition(pool,
                              [&](const auto& it)
                              {
                                  const auto& id = it.id();
                                  return id == key;
                              });
    }

    /**
     * @brief Gets assets that satisfy a condition.
     * @param predicate The predicate function to determine which assets to get.
     * @return A vector of asset handles that satisfy the condition.
     */
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

    /**
     * @brief Gets all assets in a specified group.
     * @param group The group to get assets from.
     * @return A vector of asset handles in the group.
     */
    auto get_group(const std::string& group) const -> std::vector<asset_handle<T>>
    {
        return get_with_condition(
            [&](const auto& it)
            {
                const auto& id = it.id();
                hpp::string_view id_view(id);
                return id_view.starts_with(group);
            });
    }

    /// Function for loading assets from file.
    load_from_file_t load_from_file;
    /// Function for loading assets from instance.
    load_from_instance_t load_from_instance;
    /// Container for asset requests.
    request_container_t container;
    /// Mutex for container operations.
    mutable std::recursive_mutex container_mutex;
};

} // namespace ace
