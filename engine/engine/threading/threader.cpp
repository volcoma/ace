#include "threader.h"

#include <base/platform/thread.hpp>
#include <logging/logging.h>

namespace ace
{
threader::threader()
{
    itc::init_data data{};
    data.set_thread_name = [](std::thread& thread, const std::string& name)
    {
        platform::set_thread_name(thread, name.c_str());
    };

    data.log_info = [](const std::string& msg)
    {
        APPLOG_INFO(msg);
    };
    data.log_error = [](const std::string& msg)
    {
        APPLOG_ERROR(msg);
    };

    itc::init(data);

    pool = std::make_unique<itc::thread_pool>();
}

threader::~threader()
{
    pool.reset();
    itc::shutdown();
}

auto threader::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

auto threader::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

void threader::process()
{
    itc::this_thread::process();
}

} // namespace ace
