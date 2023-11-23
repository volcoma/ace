#include "logging.h"

#include <base/platform/config.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#if ACE_ON(ACE_PLATFORM_WINDOWS)
#include <spdlog/sinks/msvc_sink.h>
namespace spdlog
{
namespace sinks
{
using platform_sink_mt = wincolor_stdout_sink_mt;
using platform_sink_st = wincolor_stdout_sink_st;
} // namespace sinks
} // namespace spdlog
#elif ACE_ON(ACE_PLATFORM_LINUX) || ACE_ON(ACE_PLATFORM_APPLE)
#include <spdlog/sinks/ansicolor_sink.h>
namespace spdlog
{
namespace sinks
{
using platform_sink_mt = ansicolor_stdout_sink_mt;
using platform_sink_st = ansicolor_stdout_sink_st;
} // namespace sinks
} // namespace spdlog
#elif ACE_ON(ACE_PLATFORM_ANDROID)
#include <spdlog/sinks/android_sink.h>
namespace spdlog
{
namespace sinks
{
using platform_sink_mt = android_sink_mt;
using platform_sink_st = android_sink_st;
} // namespace sinks
} // namespace spdlog
#else
#include <spdlog/sinks/ansicolor_sink.h>
namespace spdlog
{
namespace sinks
{
using platform_sink_mt = stdout_sink_mt;
using platform_sink_st = stdout_sink_st;
} // namespace sinks
} // namespace spdlog
#endif


namespace ace
{

auto get_mutable_logging_container() -> std::shared_ptr<spdlog::sinks::dist_sink_mt>
{
	static auto sink = std::make_shared<spdlog::sinks::dist_sink_mt>();
	return sink;
}

logging::logging(const std::string& output_file)
{
	auto logging_container = get_mutable_logging_container();
	auto console_sink = std::make_shared<spdlog::sinks::platform_sink_mt>();
    console_sink->set_level(spdlog::level::trace);

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(output_file, true);
    file_sink->set_level(spdlog::level::trace);

	logging_container->add_sink(console_sink);
	logging_container->add_sink(file_sink);
    logging_container->set_level(spdlog::level::trace);

    auto logger = std::make_shared<spdlog::logger>(APPLOG, logging_container);
    spdlog::initialize_logger(logger);
    spdlog::set_level(spdlog::level::info);
}

logging::~logging()
{
    spdlog::shutdown();
}
} // namespace ace
