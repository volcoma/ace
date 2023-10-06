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

    virtual void clear(itc::thread_pool& pool) = 0;
    virtual void clear(itc::thread_pool& pool, const std::string& group) = 0;
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
    //-----------------------------------------------------------------------------
    //  Name : ~storage ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    ~asset_storage() override = default;

    void clear_with_condition(const predicate_t& predicate)
    {
        std::lock_guard<std::recursive_mutex> lock(container_mutex);
        for(auto it = container.cbegin(); it != container.cend();)
        {
            if(predicate(*it))
            {
                it = container.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    //-----------------------------------------------------------------------------
    //  Name : clear ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void clear(itc::thread_pool& pool) final
    {
        clear_with_condition(
            [&](const auto& it)
            {
                const auto& handle = it.second;
                pool.stop(handle.task_id());
                return true;
            });
    }

    //-----------------------------------------------------------------------------
    //  Name : clear ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void clear(itc::thread_pool& pool, const std::string& group) final
    {
        clear_with_condition(
            [&](const auto& it)
            {
                const auto& id = it.first;
                const auto& handle = it.second;

                hpp::string_view id_view(id);
                if(id_view.starts_with(group))
                {
                    pool.stop(handle.task_id());
                    return true;
                }
                return false;
            });
    }

    /// key, mode
    load_from_file_t load_from_file;

    /// key, mode
    load_from_instance_t load_from_instance;

    /// Storage container
    request_container_t container;

    /// Mutex
    std::recursive_mutex container_mutex;
};
} // namespace ace
