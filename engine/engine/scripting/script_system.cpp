#include "script_system.h"

#include <monort/monort.h>
#include <monopp/mono_jit.h>
#include <logging/logging.h>
#include <filesystem/filesystem.h>

namespace ace
{
auto find_mono() -> mono::compiler_paths
{
    mono::compiler_paths result;
    {
        const auto& names = mono::get_common_library_names();
        const auto& paths = mono::get_common_library_paths();

        auto found_library = fs::find_library(names, paths);

        result.assembly_dir = fs::absolute(found_library.parent_path()).string();
        result.config_dir = fs::absolute(fs::path(result.assembly_dir) / ".." / "etc").string();

    }

    {
        const auto& names = mono::get_common_executable_names();
        const auto& paths = mono::get_common_executable_paths();


        result.msc_executable = fs::find_program(names, paths).string();

    }

    return result;
}

script_system::script_system()
{

}

script_system::~script_system()
{

}

auto script_system::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return mono::init(find_mono(), true);
}

auto script_system::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    mono::shutdown();

    return true;
}

} // namespace ace
