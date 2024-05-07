#include "asset_compiler.h"
#include "importers/mesh_importer.h"

#include <bx/error.h>
#include <bx/process.h>
#include <bx/string.h>

#include <base/platform/config.hpp>
#include <graphics/shader.h>
#include <graphics/texture.h>
#include <logging/logging.h>
#include <uuid/uuid.h>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

// #include <runtime/ecs/constructs/prefab.h>
// #include <runtime/ecs/constructs/scene.h>
// #include <runtime/meta/audio/sound.hpp>
#include <engine/meta/animation/animation.hpp>
#include <engine/meta/ecs/entity.hpp>
#include <engine/meta/physics/physics_material.hpp>
#include <engine/meta/rendering/material.hpp>
#include <engine/meta/rendering/mesh.hpp>
#include <engine/meta/audio/audio_clip.hpp>

#include <array>
#include <fstream>

namespace ace::asset_compiler
{

namespace
{
auto resolve_path(const std::string& key) -> fs::path
{
    return fs::absolute(fs::resolve_protocol(key));
}

auto resolve_input_file(const fs::path& key) -> fs::path
{
    fs::path absolute_path = fs::convert_to_protocol(key);
    absolute_path = fs::resolve_protocol(fs::replace(absolute_path, ":/meta", ":/data"));
    if(absolute_path.extension() == ".meta")
    {
        absolute_path.replace_extension();
    }
    return absolute_path;
}

auto escape_str(const std::string& str) -> std::string
{
    return "\"" + str + "\"";
}

} // namespace

auto run_process(const std::string& process, const std::vector<std::string>& args_array, std::string& err) -> bool
{
    std::string args;
    size_t i = 0;
    for(const auto& arg : args_array)
    {
        if(arg.front() == '-')
        {
            args += arg;
        }
        else
        {
            args += escape_str(arg);
        }

        if(i++ != args_array.size() - 1)
            args += " ";
    }

    bx::Error error;
    bx::ProcessReader process_reader;

    auto executable_dir = fs::resolve_protocol("binary:/");
    auto process_full = executable_dir / process;
#if ACE_ON(ACE_PLATFORM_WINDOWS)
    process_reader.open((process_full.string() + " " + args).c_str(), "", &error);
#else
    process_reader.open(process_full.string().c_str(), args.c_str(), &error);
#endif
    if(!error.isOk())
    {
        err = std::string(error.getMessage().getPtr());
        return false;
    }
    else
    {
        std::array<char, 2048 * 32> buffer;
        buffer.fill(0);
        int32_t sz = process_reader.read(buffer.data(), static_cast<std::int32_t>(buffer.size()), &error);

        process_reader.close();
        int32_t result = process_reader.getExitCode();

        if(0 != result)
        {
            err = std::string(error.getMessage().getPtr());
            if(sz > 0)
            {
                err += " " + std::string(buffer.data());
            }
            return false;
        }

        return true;
    }
}
template<>
void compile<gfx::shader>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    std::string file = absolute_path.stem().string();
    fs::path dir = absolute_path.parent_path();

    fs::path include = fs::resolve_protocol("engine:/data/shaders");
    std::string str_include = include.string();
    fs::path varying = dir / (file + ".io");
    std::string str_varying = varying.string();

    std::string str_platform;
    std::string str_profile;
    std::string str_type;
    std::string str_opt = "3";

    bool vs = hpp::string_view(file).starts_with("vs_");
    bool fs = hpp::string_view(file).starts_with("fs_");
    bool cs = hpp::string_view(file).starts_with("cs_");

    auto renderer = gfx::get_renderer_type();

    if(renderer == gfx::renderer_type::Vulkan)
    {
        str_platform = "windows";
        str_profile = "spirv";
    }

    if(renderer == gfx::renderer_type::Direct3D11 || renderer == gfx::renderer_type::Direct3D12)
    {
        str_platform = "windows";

        if(vs || fs)
        {
            str_profile = "s_5_0";
            str_opt = "3";
        }
        else if(cs)
        {
            str_profile = "s_5_0";
            str_opt = "1";
        }
    }
    else if(renderer == gfx::renderer_type::OpenGLES)
    {
        str_platform = "android";
    }
    else if(renderer == gfx::renderer_type::OpenGL)
    {
        str_platform = "linux";

        if(vs || fs)
            str_profile = "140";
        else if(cs)
            str_profile = "430";
    }
    else if(renderer == gfx::renderer_type::Metal)
    {
        str_platform = "osx";
        str_profile = "metal";
    }

    if(vs)
        str_type = "vertex";
    else if(fs)
        str_type = "fragment";
    else if(cs)
        str_type = "compute";
    else
        str_type = "unknown";

    std::vector<std::string> args_array = {
        "-f",
        str_input,
        "-o",
        str_output,
        "-i",
        str_include,
        "--varyingdef",
        str_varying,
        "--type",
        str_type,
        //        "--Werror"
    };

    if(!str_platform.empty())
    {
        args_array.emplace_back("--platform");
        args_array.emplace_back(str_platform);
    }

    if(!str_profile.empty())
    {
        args_array.emplace_back("-p");
        args_array.emplace_back(str_profile);
    }

    if(!str_opt.empty())
    {
        args_array.emplace_back("-O");
        args_array.emplace_back(str_opt);
    }

    std::string error;

    {
        std::ofstream output_file(str_output);
        (void)output_file;
    }

    if(!run_process("shaderc", args_array, error))
    {
        APPLOG_ERROR("Failed compilation of {0} with error: {1}", str_input, error);
    }
    else
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }
    fs::remove(temp, err);
}

template<>
void compile<gfx::texture>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    const std::vector<std::string> args_array = {
        "-f",
        str_input,
        "-o",
        str_output,
        "--as",
        "ktx",
        "-m",
        "-t",
        "BGRA8",
    };

    std::string error;

    {
        std::ofstream output_file(str_output);
        (void)output_file;
    }

    if(!run_process("texturec", args_array, error))
    {
        APPLOG_ERROR("Failed compilation of {0} with error: {1}", str_input, error);
    }
    else
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }
    fs::remove(temp, err);
}

template<>
void compile<material>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    std::shared_ptr<material> material;
    {
        load_from_file(str_input, material);
        save_to_file_bin(str_output, material);
    }

    if(material)
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);
}

template<>
void compile<mesh>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    fs::path file = absolute_path.stem();
    fs::path dir = absolute_path.parent_path();

    mesh::load_data data;
    std::vector<animation> animations;
    std::vector<importer::imported_material> materials;
    std::vector<importer::imported_texture> textures;

    if(!importer::load_mesh_data_from_file(am, absolute_path, dir, data, animations, materials, textures))
    {
        APPLOG_ERROR("Failed compilation of {0}", str_input);
        return;
    }
    if(!data.vertex_data.empty())
    {
        save_to_file_bin(str_output, data);

        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
        fs::remove(temp, err);
    }

    {
        for(const auto& animation : animations)
        {
            temp = fs::temp_directory_path(err);
            temp.append(hpp::to_string(generate_uuid()) + ".buildtemp");
            {
                save_to_file(temp.string(), animation);
            }

            fs::path anim_output;
            if(animation.name.empty())
            {
                anim_output = (dir / file).string() + ".anim";
            }
            else
            {
                anim_output = dir / (animation.name + ".anim");
            }

            fs::copy_file(temp, anim_output, fs::copy_options::overwrite_existing, err);
            fs::remove(temp, err);

            // APPLOG_INFO("Successful compilation of animation {0}", animation.name);
        }

        for(const auto& material : materials)
        {
            temp = fs::temp_directory_path(err);
            temp.append(hpp::to_string(generate_uuid()) + ".buildtemp");
            {
                save_to_file(temp.string(), material.mat);
            }
            fs::path mat_output;

            if(material.name.empty())
            {
                mat_output = (dir / file).string() + ".mat";
            }
            else
            {
                mat_output = dir / (material.name + ".mat");
            }

            fs::copy_file(temp, mat_output, fs::copy_options::overwrite_existing, err);
            fs::remove(temp, err);

            // APPLOG_INFO("Successful compilation of material {0}", material.name);
        }
    }
}

template<>
void compile<animation>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    animation anim;
    {
        load_from_file(str_input, anim);
        save_to_file_bin(str_output, anim);
    }

    if(!anim.channels.empty())
    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);
}

template<>
void compile<prefab>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);
    std::string str_input = absolute_path.string();

    fs::error_code er;
    fs::copy_file(absolute_path, output, fs::copy_options::overwrite_existing, er);
    APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
}

template<>
void compile<scene_prefab>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);
    std::string str_input = absolute_path.string();

    fs::error_code er;
    fs::copy_file(absolute_path, output, fs::copy_options::overwrite_existing, er);
    APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
}

template<>
void compile<physics_material>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    physics_material material;
    {
        load_from_file(str_input, material);
        save_to_file_bin(str_output, material);
    }

    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);
}


template<>
void compile<audio_clip>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    audio::sound_data clip;
    {
        std::string error;
        if(!load_from_file(str_input, clip, error))
        {
            APPLOG_ERROR("Failed compilation of {0} with error: {1}", str_input, error);
            return;
        }


        clip.convert_to_mono();
        save_to_file_bin(str_output, clip);
    }

    {
        APPLOG_INFO("Successful compilation of {0} -> {1}", str_input, output.string());
        fs::copy_file(temp, output, fs::copy_options::overwrite_existing, err);
    }

    fs::remove(temp, err);
}

} // namespace ace::asset_compiler
