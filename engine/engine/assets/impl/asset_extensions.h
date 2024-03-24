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
struct animation;
struct physics_material;
struct audio_clip;

} // namespace ace


namespace ex
{

template<typename T>
const std::vector<std::string>& get_suported_formats();

template<>
inline const std::vector<std::string>& get_suported_formats<gfx::texture>()
{
    static std::vector<std::string> formats = {".png", ".jpg", ".jpeg", ".tga", ".dds", ".ktx", ".pvr"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::mesh>()
{
    static std::vector<std::string> formats = {".gltf", ".glb", ".obj", ".fbx", ".dae", ".blend", ".3ds"};
    return formats;
}

template <>
inline const std::vector<std::string>& get_suported_formats<ace::audio_clip>()
{
    static std::vector<std::string> formats = {".ogg", ".wav", ".flac", ".mp3"};
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
    static std::vector<std::string> formats = {".mat"};
    return formats;
}

template<>
inline const std::vector<std::string>& get_suported_formats<ace::animation>()
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
    static std::vector<std::string> formats = {".phm"};
    return formats;
}

inline const std::vector<std::vector<std::string>>& get_all_formats()
{
    static const std::vector<std::vector<std::string>> types = {ex::get_suported_formats<gfx::texture>(),
                                                                ex::get_suported_formats<gfx::shader>(),
                                                                ex::get_suported_formats<ace::material>(),
                                                                ex::get_suported_formats<ace::mesh>(),
                                                                ex::get_suported_formats<ace::animation>(),
                                                                ex::get_suported_formats<ace::audio_clip>(),
                                                                ex::get_suported_formats<ace::prefab>(),
                                                                ex::get_suported_formats<ace::scene_prefab>(),
                                                                ex::get_suported_formats<ace::physics_material>()};

    return types;
}

template<typename T>
inline bool is_format(const std::string& ex)
{
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

} // namespace ex
