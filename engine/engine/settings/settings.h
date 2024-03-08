#pragma once


#include <base/basetypes.hpp>
#include <string>

namespace ace
{

struct settings
{

    struct app_settings
    {
        std::string company;
        std::string product;
        std::string version;
    };


    struct graphics_settings
    {

    };

    struct standalone_settings
    {
    };
};
} // namespace ace
