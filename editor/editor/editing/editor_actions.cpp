#include "editor_actions.h"

#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/assets/impl/asset_reader.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <engine/meta/assets/asset_database.hpp>
#include <engine/meta/ecs/entity.hpp>

#include <editor/editing/editing_manager.h>
#include <editor/system/project_manager.h>

#include <filedialog/filedialog.h>
#include <filesystem/filesystem.h>
#include <subprocess/subprocess.hpp>

#include <base/platform/config.hpp>
#include <string_utils/utils.h>

namespace ace
{

namespace
{

auto get_vscode_executable() -> fs::path
{
    fs::path executablePath;

#ifdef _WIN32
    // Windows implementation
    try
    {
        // Common installation paths
        std::vector<fs::path> possiblePaths = {"C:\\Program Files\\Microsoft VS Code\\Code.exe",
                                               "C:\\Program Files (x86)\\Microsoft VS Code\\Code.exe",
                                               fs::path(std::getenv("LOCALAPPDATA")) / "Programs" /
                                                   "Microsoft VS Code" / "Code.exe"};

        for(const auto& path : possiblePaths)
        {
            if(fs::exists(path))
            {
                executablePath = path;
                break;
            }
        }

        if(executablePath.empty())
        {
            // Search for Code.exe in the PATH environment variable
            const char* pathEnv = std::getenv("PATH");
            if(pathEnv)
            {
                std::string pathEnvStr(pathEnv);
                std::stringstream ss(pathEnvStr);
                std::string token;
                while(std::getline(ss, token, ';'))
                {
                    fs::path codePath = fs::path(token) / "Code.exe";
                    if(fs::exists(codePath))
                    {
                        executablePath = codePath;
                        break;
                    }
                }
            }

            // If still not found, perform a recursive search in Program Files
            if(executablePath.empty())
            {
                std::vector<fs::path> directoriesToSearch = {"C:\\Program Files",
                                                             "C:\\Program Files (x86)",
                                                             fs::path(std::getenv("LOCALAPPDATA")) / "Programs"};

                for(const auto& dir : directoriesToSearch)
                {
                    try
                    {
                        for(const auto& entry : fs::recursive_directory_iterator(dir))
                        {
                            if(entry.is_regular_file() && entry.path().filename() == "Code.exe")
                            {
                                executablePath = entry.path();
                                break;
                            }
                        }
                        if(!executablePath.empty())
                        {
                            break;
                        }
                    }
                    catch(const fs::filesystem_error&)
                    {
                        continue;
                    }
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error finding VSCode executable path on Windows: " << e.what() << std::endl;
    }

#elif __APPLE__
    // macOS implementation
    try
    {
        // Common application bundle paths
        std::vector<fs::path> possibleAppPaths = {"/Applications/Visual Studio Code.app",
                                                  "/Applications/Visual Studio Code - Insiders.app",
                                                  fs::path(std::getenv("HOME")) / "Applications" /
                                                      "Visual Studio Code.app"};

        for(const auto& appPath : possibleAppPaths)
        {
            if(fs::exists(appPath))
            {
                // The executable is inside the app bundle
                fs::path codeExecutable = appPath / "Contents" / "MacOS" / "Electron";
                if(fs::exists(codeExecutable))
                {
                    executablePath = codeExecutable;
                    break;
                }
            }
        }

        if(executablePath.empty())
        {
            // Search for 'code' in /usr/local/bin or /usr/bin
            std::vector<fs::path> possibleLinks = {"/usr/local/bin/code", "/usr/bin/code"};
            for(const auto& linkPath : possibleLinks)
            {
                if(fs::exists(linkPath))
                {
                    // Resolve symlink
                    executablePath = fs::canonical(linkPath);
                    break;
                }
            }
        }

        if(executablePath.empty())
        {
            // Search in PATH environment variable
            const char* pathEnv = std::getenv("PATH");
            if(pathEnv)
            {
                std::string pathEnvStr(pathEnv);
                std::stringstream ss(pathEnvStr);
                std::string token;
                while(std::getline(ss, token, ':'))
                {
                    fs::path codePath = fs::path(token) / "code";
                    if(fs::exists(codePath))
                    {
                        executablePath = fs::canonical(codePath);
                        break;
                    }
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error finding VSCode executable path on macOS: " << e.what() << std::endl;
    }

#elif __linux__
    // Linux implementation
    try
    {
        // Search for 'code' executable in PATH
        const char* pathEnv = std::getenv("PATH");
        if(pathEnv)
        {
            std::string pathEnvStr(pathEnv);
            std::stringstream ss(pathEnvStr);
            std::string token;
            while(std::getline(ss, token, ':'))
            {
                fs::path codePath = fs::path(token) / "code";
                if(fs::exists(codePath) && fs::is_regular_file(codePath))
                {
                    // Resolve symlink if necessary
                    executablePath = fs::canonical(codePath);
                    break;
                }
            }
        }

        if(executablePath.empty())
        {
            // Check common installation directories
            std::vector<fs::path> possiblePaths = {
                "/usr/share/code/bin/code",
                "/usr/share/code-insiders/bin/code",
                "/usr/local/share/code/bin/code",
                "/opt/visual-studio-code/bin/code",
                "/var/lib/flatpak/app/com.visualstudio.code/current/active/files/bin/code",
                fs::path(std::getenv("HOME")) / ".vscode" / "bin" / "code"};

            for(const auto& path : possiblePaths)
            {
                if(fs::exists(path))
                {
                    executablePath = path;
                    break;
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error finding VSCode executable path on Linux: " << e.what() << std::endl;
    }

#else
#error "Unsupported operating system."
#endif

    return executablePath;
}

void remove_extensions(std::vector<std::vector<std::string>>& resourceExtensions,
                       const std::vector<std::string>& extsToRemove)
{
    // Convert extsToRemove to a set of lowercase strings
    std::unordered_set<std::string> extsToRemoveSet;
    for(const auto& ext : extsToRemove)
    {
        extsToRemoveSet.insert(string_utils::to_lower(ext));
    }

    for(auto outerIt = resourceExtensions.begin(); outerIt != resourceExtensions.end();)
    {
        std::vector<std::string>& innerVec = *outerIt;

        innerVec.erase(std::remove_if(innerVec.begin(),
                                      innerVec.end(),
                                      [&extsToRemoveSet](const std::string& ext)
                                      {
                                          return extsToRemoveSet.find(string_utils::to_lower(ext)) !=
                                                 extsToRemoveSet.end();
                                      }),
                       innerVec.end());

        if(innerVec.empty())
        {
            outerIt = resourceExtensions.erase(outerIt);
        }
        else
        {
            ++outerIt;
        }
    }
}

void generate_launch_json(const std::string& file_path)
{
    // Define the JSON content as a raw string literal
    const std::string json_content = R"json(
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Attach to Mono",
            "request": "attach",
            "type": "mono",
            "address": "localhost",
            "port": 55555
        }
    ]
}
)json";

    // Write the JSON string to a file
    std::ofstream file(file_path);
    if(file.is_open())
    {
        file << json_content;
    }
}

void generate_workspace_file(const std::string& file_path,
                             const std::vector<std::vector<std::string>>& exclude_extensions)
{
    // Start constructing the JSON content
    std::ostringstream json_stream;

    json_stream << "{\n";
    json_stream << "    \"folders\": [\n";
    json_stream << "        {\n";
    json_stream << "            \"path\": \"../data\"\n";
    json_stream << "        }\n";
    json_stream << "    ],\n";
    json_stream << "    \"settings\": {\n";
    json_stream << "        \"files.exclude\": {\n";
    json_stream << "            \"**/.git\": true,\n";
    json_stream << "            \"**/.svn\": true";

           // Add the exclude patterns from the provided extensions
    for (const auto& extensions : exclude_extensions)
    {
        for (const auto& ext : extensions)
        {
            // Escape any special characters in the extension if necessary

                   // Create the pattern to exclude files with the given extension
            std::string pattern = "**/*" + ext;

                   // Add a comma before each new entry
            json_stream << ",\n";
            json_stream << "            \"" << pattern << "\": true";
        }
    }

           // Close the files.exclude object and the settings object
    json_stream << "\n";
    json_stream << "        }\n"; // End of "files.exclude"
    json_stream << "    }\n";     // End of "settings"

           // Add the "launch" section
    json_stream << ",\n";
    json_stream << "    \"launch\": {\n";
    json_stream << "        \"version\": \"0.2.0\",\n";
    json_stream << "        \"configurations\": [\n";
    json_stream << "            {\n";
    json_stream << "                \"name\": \"Attach to Mono\",\n";
    json_stream << "                \"request\": \"attach\",\n";
    json_stream << "                \"type\": \"mono\",\n";
    json_stream << "                \"address\": \"localhost\",\n";
    json_stream << "                \"port\": 55555\n";
    json_stream << "            }\n";
    json_stream << "        ]\n";
    json_stream << "    }\n";

           // Close the JSON object
    json_stream << "}";

    // Write the JSON string to a file
    std::ofstream file(file_path);
    if(file.is_open())
    {
        file << json_stream.str();
    }

    APPLOG_INFO("Workspace {}", file_path);
}

auto trim_line = [](std::string& line)
{
    // Trim trailing spaces and \r
    line.erase(std::find_if(line.rbegin(),
                            line.rend(),
                            [](char ch)
                            {
                                return !std::isspace(int(ch));
                            })
                   .base(),
               line.end());
};

auto parse_line(std::string& line, const fs::path& fs_parent_path) -> bool
{
#ifdef ACE_PLATFORM_WINDOWS
    // parse dependencies output
    if(line.find("[ApplicationDirectory]") != std::string::npos)
    {
        std::size_t pos = line.find(':');
        if(pos != std::string::npos)
        {
            line = line.substr(pos + 2); // +2 to skip ": "
            trim_line(line);

            return true;
        }
    }
#else
    // parse ldd output
    size_t pos = line.find("=> ");
    if(pos != std::string::npos)
    {
        line = line.substr(pos + 3); // +3 to remove '=> '
        size_t address_pos = line.find(" (0x");
        if(address_pos != std::string::npos)
        {
            line = line.substr(0, address_pos); // remove the address
        }

        trim_line(line);

        fs::path fs_path(line);

        if(fs::exists(fs_path) && fs::exists(fs_parent_path))
        {
            if(fs::equivalent(fs_path.parent_path(), fs_parent_path))
            {
                return true;
            }
        }
    }

#endif
    return false;
}

auto get_subprocess_params(const fs::path& file) -> std::vector<std::string>
{
    std::vector<std::string> params;

#ifdef ACE_PLATFORM_WINDOWS
    params.emplace_back(fs::resolve_protocol("editor:/tools/dependencies/Dependencies.exe").string());
    params.emplace_back("-modules");
    params.emplace_back(file.string());

#else

    params.emplace_back("ldd");
    params.emplace_back(file.string());
#endif
    return params;
}

auto parse_dependencies(const std::string& input, const fs::path& fs_parent_path) -> std::vector<std::string>
{
    std::vector<std::string> dependencies;
    std::stringstream ss(input);
    std::string line;

    while(std::getline(ss, line))
    {
        if(parse_line(line, fs_parent_path))
        {
            dependencies.push_back(line);
        }
    }
    return dependencies;
}

auto get_dependencies(const fs::path& file) -> std::vector<std::string>
{
    auto parent_path = file.parent_path();

    auto params = get_subprocess_params(file);
    auto result = subprocess::call(params);
    return parse_dependencies(result.out_output, parent_path);
}

auto save_scene_impl(rtti::context& ctx, const fs::path& path) -> bool
{
    auto& ec = ctx.get<ecs>();
    save_to_file(path.string(), ec.get_scene());

    return true;
}

auto save_scene_as_impl(rtti::context& ctx, fs::path& path) -> bool
{
    std::string picked;
    if(native::save_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Save scene as",
                                fs::resolve_protocol("app:/data/").string()))
    {
        auto& em = ctx.get<editing_manager>();

        path = picked;

        if(!ex::is_format<scene_prefab>(path.extension().generic_string()))
        {
            path.replace_extension(ex::get_format<scene_prefab>(false));
        }

        return save_scene_impl(ctx, path);
    }

    return false;
}

} // namespace

auto editor_actions::new_scene(rtti::context& ctx) -> bool
{
    auto& em = ctx.get<editing_manager>();
    em.close_project();

    auto& ec = ctx.get<ecs>();
    ec.unload_scene();

    defaults::create_default_3d_scene(ctx, ec.get_scene());
    return true;
}
auto editor_actions::open_scene(rtti::context& ctx) -> bool
{
    std::string picked;
    if(native::open_file_dialog(picked,
                                ex::get_suported_formats_with_wildcard<scene_prefab>(),
                                "Scene files",
                                "Open scene",
                                fs::resolve_protocol("app:/data/").string()))
    {
        auto path = fs::convert_to_protocol(picked);
        if(ex::is_format<scene_prefab>(path.extension().generic_string()))
        {
            auto& em = ctx.get<editing_manager>();
            em.close_project();

            auto& am = ctx.get<asset_manager>();
            auto asset = am.get_asset<scene_prefab>(path.string());

            auto& ec = ctx.get<ecs>();
            ec.unload_scene();

            auto& scene = ec.get_scene();
            return scene.load_from(asset);
        }
    }
    return false;
}
auto editor_actions::save_scene(rtti::context& ctx) -> bool
{
    auto& ec = ctx.get<ecs>();
    auto& scene = ec.get_scene();

    if(!scene.source)
    {
        fs::path picked;
        if(save_scene_as_impl(ctx, picked))
        {
            auto path = fs::convert_to_protocol(picked);

            auto& am = ctx.get<asset_manager>();
            scene.source = am.get_asset<scene_prefab>(path.string());
            return true;
        }
    }
    else
    {
        auto path = fs::resolve_protocol(scene.source.id());
        save_scene_impl(ctx, path);
    }

    return true;
}
auto editor_actions::save_scene_as(rtti::context& ctx) -> bool
{
    fs::path p;
    return save_scene_as_impl(ctx, p);
}
auto editor_actions::close_project(rtti::context& ctx) -> bool
{
    auto& pm = ctx.get<project_manager>();
    pm.close_project(ctx);
    return true;
}

void editor_actions::run_project(const deploy_settings& params)
{
    auto call_params = params.deploy_location / (std::string("game") + fs::executable_extension());
    subprocess::call(call_params.string());
}

auto editor_actions::deploy_project(rtti::context& ctx, const deploy_settings& params)
    -> std::map<std::string, itc::shared_future<void>>
{
    auto& th = ctx.get<threader>();

    std::map<std::string, itc::shared_future<void>> jobs;
    std::vector<itc::shared_future<void>> jobs_seq;

    fs::error_code ec;

    auto& am = ctx.get<asset_manager>();
    // am.get_database("engine:/")

    if(params.deploy_dependencies)
    {
        APPLOG_INFO("Clearing {}", params.deploy_location.string());
        fs::remove_all(params.deploy_location, ec);
        fs::create_directories(params.deploy_location, ec);

        auto job =
            th.pool
                ->schedule(
                    [params]()
                    {
                        APPLOG_INFO("Deploying Dependencies...");

                        fs::path app_executable = fs::resolve_protocol("binary:/game" + fs::executable_extension());
                        auto deps = get_dependencies(app_executable);

                        fs::error_code ec;
                        for(const auto& dep : deps)
                        {
                            APPLOG_TRACE("Copying {} -> {}", dep, params.deploy_location.string());
                            fs::copy(dep, params.deploy_location, fs::copy_options::overwrite_existing, ec);
                        }

                        APPLOG_TRACE("Copying {} -> {}", app_executable.string(), params.deploy_location.string());
                        fs::copy(app_executable, params.deploy_location, fs::copy_options::overwrite_existing, ec);

                        APPLOG_INFO("Deploying Dependencies - Done...");
                    })
                .share();
        jobs["Deploying Dependencies"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params]()
                           {
                               APPLOG_INFO("Deploying Project Settings...");

                               auto data = fs::resolve_protocol("app:/settings");
                               fs::path dst = params.deploy_location / "data" / "app" / "settings";

                               fs::error_code ec;

                               APPLOG_TRACE("Clearing {}", dst.string());
                               fs::remove_all(dst, ec);
                               fs::create_directories(dst, ec);

                               APPLOG_TRACE("Copying {} -> {}", data.string(), dst.string());
                               fs::copy(data, dst, fs::copy_options::recursive, ec);

                               APPLOG_INFO("Deploying Project Settings - Done...");
                           })
                       .share();

        jobs["Deploying Project Settings"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params, &am]()
                           {
                               APPLOG_INFO("Deploying Project Data...");

                               fs::error_code ec;
                               {
                                   auto data = fs::resolve_protocol("app:/compiled");
                                   fs::path cached_data = params.deploy_location / "data" / "app" / "compiled";

                                   APPLOG_TRACE("Clearing {}", cached_data.string());
                                   fs::remove_all(cached_data, ec);
                                   fs::create_directories(cached_data, ec);

                                   APPLOG_TRACE("Copying {} -> {}", data.string(), cached_data.string());
                                   fs::copy(data, cached_data, fs::copy_options::recursive, ec);
                               }

                               {
                                   fs::path cached_data = params.deploy_location / "data" / "app" / "assets.pack";
                                   APPLOG_TRACE("Creating Asset Pack -> {}", cached_data.string());
                                   am.save_database("app:/", cached_data);
                               }

                               APPLOG_INFO("Deploying Project Data - Done...");
                           })
                       .share();

        jobs["Deploying Project Data"] = job;
        jobs_seq.emplace_back(job);
    }

    {
        auto job = th.pool
                       ->schedule(
                           [params, &am]()
                           {
                               APPLOG_INFO("Deploying Engine Data...");

                               fs::error_code ec;
                               {
                                   fs::path cached_data = params.deploy_location / "data" / "engine" / "compiled";
                                   auto data = fs::resolve_protocol("engine:/compiled");

                                   APPLOG_TRACE("Clearing {}", cached_data.string());
                                   fs::remove_all(cached_data, ec);
                                   fs::create_directories(cached_data, ec);

                                   APPLOG_TRACE("Copying {} -> {}", data.string(), cached_data.string());
                                   fs::copy(data, cached_data, fs::copy_options::recursive, ec);
                               }

                               {
                                   fs::path cached_data = params.deploy_location / "data" / "engine" / "assets.pack";
                                   APPLOG_TRACE("Creating Asset Pack -> {}", cached_data.string());
                                   am.save_database("engine:/", cached_data);
                               }

                               APPLOG_INFO("Deploying Engine Data - Done...");
                           })
                       .share();
        jobs["Deploying Engine Data..."] = job;
        jobs_seq.emplace_back(job);
    }

    itc::when_all(std::begin(jobs_seq), std::end(jobs_seq))
        .then(itc::this_thread::get_id(),
              [params](auto f)
              {
                  if(params.deploy_and_run)
                  {
                      run_project(params);
                  }
                  else
                  {
                      fs::show_in_graphical_env(params.deploy_location);
                  }
              });

    return jobs;
}

void editor_actions::generate_script_workspace(const std::string& project_name)
{
    fs::error_code err;

    auto workspace_folder = fs::resolve_protocol("app:/.vscode");
    fs::create_directories(workspace_folder, err);

    auto workspace_launch_file = workspace_folder / "launch.json";
    generate_launch_json(workspace_launch_file.string());

    auto formats = ex::get_all_formats();
    remove_extensions(formats, ex::get_suported_formats<gfx::shader>());
    remove_extensions(formats, ex::get_suported_formats<script>());

    auto workspace_file = workspace_folder / fmt::format("{}-workspace.code-workspace", project_name);
    generate_workspace_file(workspace_file.string(), formats);
}

void editor_actions::open_workspace_on_file(const std::string& project_name, const fs::path& file, int line)
{
    itc::async(
        [project_name, file, line]()
        {
            try
            {
                auto external_tool = get_vscode_executable();
                auto workspace_key = fmt::format("app:/.vscode/{}-workspace.code-workspace", project_name);
                auto workspace_path = fs::resolve_protocol(workspace_key);

                subprocess::call(external_tool.string(), {workspace_path.string(), "-g", fmt::format("{}:{}", file.string(), line)});
            }
            catch(const std::exception& e)
            {
                APPLOG_ERROR("Cannot open external tool for file {}", file.string());
            }
        });
}

} // namespace ace
