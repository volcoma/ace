#pragma once

#include <memory>
#include <string>
#include <hpp/filesystem.hpp>
#include <logging/logging.h>

#include "../threading/threader.h"

template<typename T>
using task_future = itc::job_shared_future<T>;

template <typename T>
struct asset_link
{
    using task_future_t = task_future<std::shared_ptr<T>>;

	std::string id;
    std::string name;
	task_future_t task;
};


template <typename T>
struct asset_handle
{
    using asset_link_t = asset_link<T>;

    auto operator==(const asset_handle& rhs) const -> bool
    {
        return id() == rhs.id() && is_valid() == rhs.is_valid();
    }
    operator bool() const
    {
        return is_valid();
    }

	auto id() const -> const std::string&
	{
		return link_->id;
	}

    auto name() const -> const std::string&
	{
		return link_->name;
	}

    auto get(bool wait = true) const -> const T&
    {
        bool valid = link_->task.valid();
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
                return *value;
            }
        }

        static const T empty{};
        return empty;
    }

    auto get_ptr(bool wait = true) const -> std::shared_ptr<T>
    {
        bool valid = link_->task.valid();
        bool ready = is_ready();
        bool should_get = ready || (!ready && wait);

        if(valid && should_get)
        {
            auto value = link_->task.get();

            if(value)
            {
                return value;
            }
        }

        static const std::shared_ptr<T> empty{};
        return empty;
    }

    auto is_valid() const -> bool
    {
        return link_->task.valid();
    }

    auto is_ready() const -> bool
    {
        return link_->task.valid() && link_->task.is_ready();
    }

    auto task_id() const
    {
        return link_->task.id;
    }

    void set_internal_job(const typename asset_link_t::task_future_t& future)
    {
        link_->task = future;
    }

    void set_internal_id(const std::string& internal_id)
    {
        link_->id = internal_id;

        link_->name = fs::path(internal_id).stem().string();
    }

    void invalidate()
    {
        if(link_->task.valid())
        {
            auto task_count = link_->task.use_count();
            if(task_count > 1)
            {
                APPLOG_TRACE("{} - task leak use_count {}", link_->id, task_count);
            }
        }
        set_internal_id({});
        set_internal_job({});
    }

    static auto get_empty() -> const asset_handle&
    {
        static const asset_handle none_asset = []()
        {
            asset_handle asset;
            asset.set_internal_id("None");
            return asset;
        }();
        return none_asset;
    }

private:
    std::shared_ptr<asset_link_t> link_ = std::make_shared<asset_link_t>();
};
