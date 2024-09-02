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

#include <engine/meta/animation/animation.hpp>
#include <engine/meta/ecs/entity.hpp>
#include <engine/meta/physics/physics_material.hpp>
#include <engine/meta/rendering/material.hpp>
#include <engine/meta/rendering/mesh.hpp>
#include <engine/meta/audio/audio_clip.hpp>

#include <subprocess/subprocess.hpp>

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

auto run_process(const std::string& process, const std::vector<std::string>& args_array, std::string& err) -> bool
{
    auto result = subprocess::call(process, args_array);

    err = result.out_output;

    if(err.find("error") != std::string::npos)
    {
        return false;
    }

    return result.retcode == 0;
}
} // namespace

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

    if(!fs::exists(varying, err))
    {
        varying = dir / "varying.def.io";
    }
    std::string str_varying = varying.string();

    std::string str_platform;
    std::string str_profile;
    std::string str_type;
    std::string str_opt = "3";

    bool vs = hpp::string_view(file).starts_with("vs_");
    bool fs = hpp::string_view(file).starts_with("fs_");
    bool cs = hpp::string_view(file).starts_with("cs_");

    auto extension = output.extension();
    auto renderer = gfx::get_renderer_based_on_filename_extension(extension.string());

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
        str_profile = "100_es";

    }
    else if(renderer == gfx::renderer_type::OpenGL)
    {
        str_platform = "linux";

        if(vs || fs)
            str_profile = "120";
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
        "--define",
        "BGFX_CONFIG_MAX_BONES=" + std::to_string(gfx::get_max_blend_transforms())
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
    std::vector<animation_clip> animations;
    std::vector<importer::imported_material> materials;
    std::vector<importer::imported_texture> textures;

    if(!importer::load_mesh_data_from_file(am, absolute_path, data, animations, materials, textures))
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
void compile<animation_clip>(asset_manager& am, const fs::path& key, const fs::path& output)
{
    auto absolute_path = resolve_input_file(key);

    std::string str_input = absolute_path.string();

    fs::error_code err;
    fs::path temp = fs::temp_directory_path(err);
    temp /= hpp::to_string(generate_uuid()) + ".buildtemp";

    std::string str_output = temp.string();

    animation_clip anim;
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

    auto material = std::make_shared<physics_material>();
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
