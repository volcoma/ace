#pragma once

#include <base/basetypes.hpp>

#include <filesystem/filesystem.h>

namespace ace
{
struct deploy_settings
{
    fs::path deploy_location{};
    bool deploy_dependencies{true};
    bool deploy_and_run{};
};


} // namespace ace
