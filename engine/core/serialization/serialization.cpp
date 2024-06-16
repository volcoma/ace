#include "serialization.h"

namespace serialization
{
static std::function<void(const std::string&, const hpp::source_location& loc)> warning_logger;
void set_warning_logger(const std::function<void(const std::string&, const hpp::source_location& loc)>& logger)
{
    warning_logger = logger;
}
void log_warning(const std::string& log_msg, const hpp::source_location& loc)
{
    if(warning_logger)
        warning_logger(log_msg, loc);
}
} // namespace serialization
