#include "profiler.h"

namespace ace
{

auto get_app_profiler() -> performance_profiler*
{
    static performance_profiler profiler;
    return &profiler;
}

} // namespace ace
