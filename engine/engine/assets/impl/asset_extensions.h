#pragma once
#include <string>
#include <vector>

namespace gfx
{
struct texture;
struct shader;
} // namespace gfx

namespace ace
{
class mesh;
class material;
struct prefab;
struct scene_prefab;
struct animation_clip;
struct physics_material;
struct audio_clip;

} // namespace ace


namespace ex
{

template<typename T>
const std::vector<std::string>& get_suported_formats();

template<typename T>
const std::vector<std::string>& get_suported_dependencies_formats();


template<>
inline const std::vector<std::string>& get_suported_formats<gfx::texture>()
{
    static std::vector<std::string> formats = {".etex", ".png", ".jpg", ".jpeg", ".tga", ".dds", ".ktx", ".pvr"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::mesh>()
{
    static std::vector<std::string> formats = {".emesh", ".gltf", ".glb", ".obj", ".fbx", ".dae", ".blend", ".3ds"};
    return formats;
}

template <>
inline const std::vector<std::string>& get_suported_formats<ace::audio_clip>()
{
    static std::vector<std::string> formats = {".eaudioclip", ".ogg", ".wav", ".flac", ".mp3"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_dependencies_formats<gfx::shader>()
{
    static std::vector<std::string> formats = {".sh"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<gfx::shader>()
{
    static std::vector<std::string> formats = {".sc"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::material>()
{
    static std::vector<std::string> formats = {".mat", ".ematerial"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::animation_clip>()
{
    static std::vector<std::string> formats = {".anim"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::prefab>()
{
    static std::vector<std::string> formats = {".pfb"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::scene_prefab>()
{
    static std::vector<std::string> formats = {".spfb"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::physics_material>()
{
    static std::vector<std::string> formats = {".phm", ".ephmaterial"};
    return formats;
}

inline const std::vector<std::vector<std::string>>& get_all_formats()
{
    static const std::vector<std::vector<std::string>> types = {ex::get_suported_formats<gfx::texture>(),
                                                                ex::get_suported_formats<gfx::shader>(),
                                                                ex::get_suported_formats<ace::material>(),
                                                                ex::get_suported_formats<ace::mesh>(),
                                                                ex::get_suported_formats<ace::animation_clip>(),
                                                                ex::get_suported_formats<ace::audio_clip>(),
                                                                ex::get_suported_formats<ace::prefab>(),
                                                                ex::get_suported_formats<ace::scene_prefab>(),
                                                                ex::get_suported_formats<ace::physics_material>()};

    return types;
}

template<typename T>
inline bool is_format(const std::string& ex)
{
    if(ex.empty())
    {
        return false;
    }

    const auto& supported = ex::get_suported_formats<T>();
    return std::find_if(std::begin(supported),
                        std::end(supported),
                        [ex](const std::string& el)
                        {
                            return el.find(ex) != std::string::npos;
                        }) != std::end(supported);
}

template<typename T>
inline auto get_format(bool include_dot = true) -> std::string
{
    auto format = get_suported_formats<T>().front();
    if(include_dot)
    {
        return format;
    }
    return format.substr(1);
}

template<typename T>
inline auto get_suported_formats_with_wildcard() -> std::vector<std::string>
{
    auto formats = get_suported_formats<T>();
    for(auto& fmt : formats)
    {
        fmt.insert(fmt.begin(), '*');
    }

    return formats;
}

inline auto get_type(const std::string& ex) -> const std::string&
{
    if(is_format<gfx::texture>(ex))
    {
        static const std::string result = "Texture";
        return result;
    }
    else if(is_format<gfx::shader>(ex))
    {
        static const std::string result = "Shader";
        return result;
    }
    else if(is_format<ace::material>(ex))
    {
        static const std::string result = "Material";
        return result;
    }
    else if(is_format<ace::mesh>(ex))
    {
        static const std::string result = "Mesh";
        return result;
    }
    else if(is_format<ace::animation_clip>(ex))
    {
        static const std::string result = "Animation Clip";
        return result;
    }
    else if(is_format<ace::audio_clip>(ex))
    {
        static const std::string result = "Audio Clip";
        return result;
    }
    else if(is_format<ace::prefab>(ex))
    {
        static const std::string result = "Prefab";
        return result;
    }
    else if(is_format<ace::scene_prefab>(ex))
    {
        static const std::string result = "Scene";
        return result;
    }
    else if(is_format<ace::physics_material>(ex))
    {
        static const std::string result = "Physics Material";
        return result;
    }

    static const std::string result = "";
    return result;
}

} // namespace ex
