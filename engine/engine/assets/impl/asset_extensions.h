#pragma once
#include <string>
#include <vector>



namespace gfx
{
struct texture;
struct shader;
}

namespace ace
{
class mesh;
//struct prefab;
//struct scene;
class material;
struct animation;
}



//namespace audio
//{
//class sound;
//}


namespace ex
{

template <typename T>
const std::vector<std::string>& get_suported_formats();

template <>
inline const std::vector<std::string>& get_suported_formats<gfx::texture>()
{
	static std::vector<std::string> formats = {".png", ".jpg", ".jpeg", ".tga", ".dds", ".ktx", ".pvr"};
	return formats;
}

template <>
inline const std::vector<std::string>& get_suported_formats<ace::mesh>()
{
	static std::vector<std::string> formats = {".gltf", ".glb", ".obj", ".fbx", ".dae", ".blend", ".3ds"};
	return formats;
}

//template <>
//inline const std::vector<std::string>& get_suported_formats<audio::sound>()
//{
//	static std::vector<std::string> formats = {".ogg", ".wav"};
//	return formats;
//}

template <>
inline const std::vector<std::string>& get_suported_formats<gfx::shader>()
{
	static std::vector<std::string> formats = {".sc"};
	return formats;
}

template <>
inline const std::vector<std::string>& get_suported_formats<ace::material>()
{
	static std::vector<std::string> formats = {".mat"};
	return formats;
}

template <>
inline const std::vector<std::string>& get_suported_formats<ace::animation>()
{
	static std::vector<std::string> formats = {".anim"};
	return formats;
}

//template <>
//inline const std::vector<std::string>& get_suported_formats<prefab>()
//{
//	static std::vector<std::string> formats = {".pfb"};
//	return formats;
//}

//template <>
//inline const std::vector<std::string>& get_suported_formats<scene>()
//{
//	static std::vector<std::string> formats = {".sgr"};
//	return formats;
//}

inline const std::vector<std::vector<std::string>>& get_all_formats()
{
	static const std::vector<std::vector<std::string>> types = {
		ex::get_suported_formats<gfx::texture>(),
        ex::get_suported_formats<gfx::shader>(),
        ex::get_suported_formats<ace::material>(),
		ex::get_suported_formats<ace::mesh>(),
		ex::get_suported_formats<ace::animation>(),
//		ex::get_suported_formats<audio::sound>(),
//		ex::get_suported_formats<prefab>(),
//		ex::get_suported_formats<scene>()
    };

	return types;
}

template<typename T>
inline bool is_format(const std::string& ex)
{
    const auto& supported = ex::get_suported_formats<T>();
    return std::find(std::begin(supported), std::end(supported), ex) != std::end(supported);
}
}
