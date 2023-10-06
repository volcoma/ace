#pragma once

#include <memory>
#include <string>
#include "../threading/threader.h"


template<typename T>
using task_future = itc::job_shared_future<T>;

template <typename T>
struct asset_link
{
    using task_future_t = task_future<std::shared_ptr<T>>;

	std::string id;
	task_future_t task;
};


template <typename T>
struct asset_handle
{
    using asset_link_t = asset_link<T>;

	auto id() const -> const std::string&
	{
		return link_->id;
	}

    auto get() const -> const T&
    {
        if(link_->task.valid())
        {
            auto value = link_->task.get();

            if(value)
            {
                return *value;
            }
        }

        static const T empty{};
        return empty;
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
    }

    void invalidate()
    {
        set_internal_id({});
        set_internal_job({});
    }

private:
    std::shared_ptr<asset_link_t> link_ = std::make_shared<asset_link_t>();
};
