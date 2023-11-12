#pragma once

#include <string>

namespace version
{
    std::string get_major();
    std::string get_minor();
    std::string get_patch();

    std::string get_full();
}
