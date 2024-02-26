#pragma once

#include <spdlog/sinks/callback_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/spdlog.h>

namespace ace
{
using namespace spdlog;

#define APPLOG               "Log"
#define APPLOG_TRACE(...)    SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::trace, __VA_ARGS__)
#define APPLOG_INFO(...)     SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::info, __VA_ARGS__)
#define APPLOG_WARNING(...)  SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::warn, __VA_ARGS__)
#define APPLOG_ERROR(...)    SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::err, __VA_ARGS__)
#define APPLOG_CRITICAL(...) SPDLOG_LOGGER_CALL(spdlog::get(APPLOG), spdlog::level::critical, __VA_ARGS__)

#ifndef SPDLOG_NO_SOURCE_LOC
#define SPDLOG_LOGGER_CALL_LOC(logger, file, line, func, level, ...)                                                   \
    (logger)->log(spdlog::source_loc{file, line, func}, level, __VA_ARGS__)
#else
#define SPDLOG_LOGGER_CALL_LOC(logger, level, ...) (logger)->log(spdlog::source_loc{}, level, __VA_ARGS__)
#endif
#define APPLOG_TRACE_LOC(FILE_LOC, LINE_LOC, FUNC_LOC, ...)                                                            \
    SPDLOG_LOGGER_CALL_LOC(spdlog::get(APPLOG), FILE_LOC, LINE_LOC, FUNC_LOC, spdlog::level::trace, __VA_ARGS__)
#define APPLOG_INFO_LOC(FILE_LOC, LINE_LOC, FUNC_LOC, ...)                                                             \
    SPDLOG_LOGGER_CALL_LOC(spdlog::get(APPLOG), FILE_LOC, LINE_LOC, FUNC_LOC, spdlog::level::info, __VA_ARGS__)
#define APPLOG_WARNING_LOC(FILE_LOC, LINE_LOC, FUNC_LOC, ...)                                                          \
    SPDLOG_LOGGER_CALL_LOC(spdlog::get(APPLOG), FILE_LOC, LINE_LOC, FUNC_LOC, spdlog::level::warn, __VA_ARGS__)
#define APPLOG_ERROR_LOC(FILE_LOC, LINE_LOC, FUNC_LOC, ...)                                                            \
    SPDLOG_LOGGER_CALL_LOC(spdlog::get(APPLOG), FILE_LOC, LINE_LOC, FUNC_LOC, spdlog::level::err, __VA_ARGS__)
#define APPLOG_CRITICAL_LOC(FILE_LOC, LINE_LOC, FUNC_LOC, ...)                                                         \
    SPDLOG_LOGGER_CALL_LOC(spdlog::get(APPLOG), FILE_LOC, LINE_LOC, FUNC_LOC, spdlog::level::critical, __VA_ARGS__)

auto get_mutable_logging_container() -> std::shared_ptr<spdlog::sinks::dist_sink_mt>;

struct logging
{
    logging(const std::string& output_file = "Log.txt");
    ~logging();
};

} // namespace ace
