#pragma once

#include <hpp/filesystem.hpp>
#include <logging/logging.h>
#include <memory>
#include <string>
#include <uuid/uuid.h>

#include "../threading/threader.h"

template<typename T>
using task_future = itc::job_shared_future<T>;

template<typename T>
struct asset_link
{
    using task_future_t = task_future<std::shared_ptr<T>>;
    using weak_asset_t = std::weak_ptr<T>;

    hpp::uuid uid{};
    std::string id{};
    task_future_t task{};
    weak_asset_t weak_asset{};
};

template<typename T>
struct asset_handle
{
    using asset_link_t = asset_link<T>;

    auto operator==(const asset_handle& rhs) const -> bool
    {
        return uid() == rhs.uid() && id() == rhs.id() && is_valid() == rhs.is_valid();
    }
    operator bool() const
    {
        return is_valid();
    }

    auto id() const -> const std::string&
    {
        if(link_)
        {
            return link_->id;
        }

        static const std::string empty;
        return empty;
    }

    auto uid() const -> const hpp::uuid&
    {
        if(link_)
        {
            return link_->uid;
        }

        static const hpp::uuid empty;
        return empty;
    }

    auto name() const -> std::string
    {
        return fs::path(id()).stem().string();
    }

    auto get(bool wait = true) const -> std::shared_ptr<T>
    {
        if(link_ && !link_->weak_asset.expired())
        {
            return link_->weak_asset.lock();
        }

        bool valid = is_valid();
        bool ready = is_ready();
        bool should_get = ready || (!ready && wait);

        if(valid && should_get)
        {
            if(!ready)
            {
                link_->task.change_priority(itc::priority::high());
            }

            auto value = link_->task.get();

            if(value)
            {
                link_->weak_asset = value;
                return value;
            }
        }

        static const std::shared_ptr<T> empty = std::make_shared<T>();
        return empty;
    }

    auto is_valid() const -> bool
    {
        return link_ && link_->task.valid();
    }

    auto is_ready() const -> bool
    {
        return is_valid() && link_->task.is_ready();
    }

    auto task_id() const
    {
        if(link_)
        {
            return link_->task.id;
        }

        return itc::job_id{};
    }

    void set_internal_job(const typename asset_link_t::task_future_t& future)
    {
        ensure();
        link_->task = future;
        link_->weak_asset = {};
    }

    void set_internal_ids(const hpp::uuid& internal_uid, const std::string& internal_id = get_empty_id())
    {
        ensure();
        link_->uid = internal_uid;
        link_->id = internal_id;
    }

    void set_internal_id(const std::string& internal_id = get_empty_id())
    {
        ensure();
        link_->id = internal_id;
    }

    void invalidate()
    {
        if(is_valid())
        {
            auto task_count = link_->task.use_count();
            if(task_count > 1)
            {
                APPLOG_TRACE("{} - task leak use_count {}", id(), task_count);
            }
        }
        set_internal_ids({});
        set_internal_job({});
    }

    static auto get_empty() -> const asset_handle&
    {
        static const asset_handle none_asset = []()
        {
            asset_handle asset;
            asset.set_internal_ids({});
            return asset;
        }();
        return none_asset;
    }

    static auto get_empty_id() -> const std::string&
    {
        static const std::string empty{"None"};
        return empty;
    }

    void ensure()
    {
        static_assert(sizeof(asset_link_t) >= 1, "Type must be fully defined");
        if(!link_)
        {
            link_ = std::make_shared<asset_link_t>();
        }
    }

private:
    std::shared_ptr<asset_link_t> link_;
};
