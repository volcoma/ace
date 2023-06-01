#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/dist_sink.h>

namespace ace
{
#define APPLOG "Log"
#define APPLOG_INFO(...) spdlog::get(APPLOG)->info(__VA_ARGS__)
#define APPLOG_TRACE(...) spdlog::get(APPLOG)->trace(__VA_ARGS__)
#define APPLOG_ERROR(...) spdlog::get(APPLOG)->error(__VA_ARGS__)
#define APPLOG_WARNING(...) spdlog::get(APPLOG)->warn(__VA_ARGS__)
#define APPLOG_NOTICE(...) spdlog::get(APPLOG)->notice(__VA_ARGS__)


auto get_mutable_logging_container() -> std::shared_ptr<spdlog::sinks::dist_sink_mt>;

struct logging
{
	logging(const std::string& output_file = "Log.txt");
    ~logging();
};

} // namespace ace
