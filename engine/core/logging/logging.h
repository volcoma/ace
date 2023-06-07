#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/callback_sink.h>

namespace ace
{
using namespace spdlog;

#define APPLOG "Log"
#define APPLOG_INFO(...) SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::info, __VA_ARGS__)
#define APPLOG_TRACE(...) SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::trace, __VA_ARGS__)
#define APPLOG_ERROR(...) SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::err, __VA_ARGS__)
#define APPLOG_WARNING(...) SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::warn, __VA_ARGS__)


auto get_mutable_logging_container() -> std::shared_ptr<spdlog::sinks::dist_sink_mt>;

struct logging
{
	logging(const std::string& output_file = "Log.txt");
    ~logging();
};

} // namespace ace
