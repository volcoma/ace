#include "version.h"

namespace version
{
    std::string get_major()
    {
        return VERSION_MAJOR;
    }

    std::string get_minor()
    {
        return VERSION_MINOR;
    }

    std::string get_patch()
    {
        return VERSION_PATCH;
    }

    std::string get_full()
    {
        return VERSION;
    }
}
