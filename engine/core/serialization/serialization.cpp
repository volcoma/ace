#include "serialization.h"

namespace serialization
{
namespace
{
auto get_warning_logger() -> log_callback_t&
{
    static log_callback_t logger;
    return logger;
}
} // namespace
void set_warning_logger(const std::function<void(const std::string&, const hpp::source_location& loc)>& logger)
{
    get_warning_logger() = logger;
}
void log_warning(const std::string& log_msg, const hpp::source_location& loc)
{
    auto& logger = get_warning_logger();
    if(logger)
        logger(log_msg, loc);
}
} // namespace serialization
