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
        std::vector<std::string> names = {"mono-2.0", "monosgen-2.0", "mono-2.0-sgen"};

        std::vector<fs::path> paths = {"C:/Program Files/Mono/lib",
                                       "/usr/lib64",
                                       "/usr/lib",
                                       "/usr/local/lib64",
                                       "/usr/local/lib",
                                       "/opt/local/lib"};

        auto found_library = fs::find_library(names, paths);


        result.assembly_dir = fs::absolute(found_library.parent_path()).string();
        result.config_dir = fs::absolute(fs::path(result.assembly_dir) / ".." / "etc").string();

    }

    {
        std::vector<fs::path> paths = {
            "C:/Program Files/Mono/bin",
            "/bin",
            "/usr/bin",
            "/usr/local/bin"
        };

        std::string program = "mcs";
#ifdef _WIN32
        program.append(".bat");
#endif
        result.msc_executable = fs::find_program(program, paths).string();

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
