#include "asset_reader.h"
#include <logging/logging.h>

// #include "../../ecs/constructs/prefab.h"
// #include "../../ecs/constructs/scene.h"
// #include "../../meta/animation/animation.hpp"
// #include "../../meta/audio/sound.hpp"
// #include "../../meta/rendering/material.hpp"
// #include "../../meta/rendering/mesh.hpp"
#include "../asset_manager.h"

#include <graphics/shader.h>
#include <graphics/texture.h>

// #include <core/audio/sound.h>
// #include <core/filesystem/filesystem.h>
// #include <core/graphics/index_buffer.h>
// #include <core/graphics/vertex_buffer.h>
// #include <core/serialization/associative_archive.h>
// #include <core/serialization/binary_archive.h>
// #include <core/serialization/serialization.h>
// #include <core/serialization/types/map.hpp>
// #include <core/serialization/types/vector.hpp>

#include <cstdint>

namespace ace::asset_reader
{

auto resolve_path(const std::string& key) -> fs::path
{
//    auto cache_key = fs::replace(key, ":/data", ":/cache");
    return fs::absolute(fs::resolve_protocol(key));
}

void log_missing_compiled_asset_for_key(const std::string& key)
{
    APPLOG_WARNING("Compiled asset {0} does not exist!"
                   "Falling back to raw asset.", key);
}

void log_missing_raw_asset_for_key(const std::string& key)
{
    APPLOG_ERROR("Asset {0} does not exist!", key);
}

void log_unknown_protocol_for_key(const std::string& key)
{
    APPLOG_ERROR("Asset {0} has unknown protocol!", key);
}

auto validate(const std::string& key, std::string& out) -> bool
{
    if(!fs::has_known_protocol(key))
    {
        log_unknown_protocol_for_key(key);
        return false;
    }

    auto absolute_path = resolve_path(key).string();
    auto compiled_absolute_path = absolute_path + ".asset";

    fs::error_code err;
    if(!fs::exists(compiled_absolute_path, err))
    {
        log_missing_compiled_asset_for_key(key);

        compiled_absolute_path = absolute_path;
    }

    if(!fs::exists(compiled_absolute_path, err))
    {
        log_missing_raw_asset_for_key(key);
        return false;
    }

    out = compiled_absolute_path;
    return true;
}

template<>
auto load_from_file<gfx::texture>(itc::thread_pool& pool,
                                  asset_handle<gfx::texture>& output,
                                  const std::string& key) -> bool
{

    std::string compiled_absolute_path{};

    if(!validate(key, compiled_absolute_path))
    {
        return false;
    }

    auto create_resource_func = [compiled_absolute_path]()
    {
        return std::make_shared<gfx::texture>(compiled_absolute_path.c_str());
    };


    auto job = pool.schedule(create_resource_func).share();

    output.set_internal_job(job);

    return true;
}

template<>
auto load_from_file<gfx::shader>(itc::thread_pool& pool,
                                 asset_handle<gfx::shader>& output,
                                 const std::string& key) -> bool
{

    std::string compiled_absolute_path{};

    if(!validate(key, compiled_absolute_path))
    {
        return false;
    }

    auto create_resource_func = [compiled_absolute_path]()
    {
        auto stream = std::ifstream{compiled_absolute_path, std::ios::in | std::ios::binary};
        auto read_memory = fs::read_stream(stream);
        return std::make_shared<gfx::shader>(read_memory);
    };

    auto job = pool.schedule(create_resource_func).share();
    output.set_internal_job(job);

    return true;
}

// template <>
// bool load_from_file<mesh>(core::task_future<asset_handle<mesh>>& output, const std::string& key)
//{
//	asset_handle<mesh> original;
//	if(output.is_ready())
//	{
//		original = output.get();
//	}

//	auto& ts = core::get_subsystem<core::task_system>();

//	auto create_resource_func_fallback = [ result = original, key ]() mutable
//	{
//		result.link->id = key;
//		return result;
//	};

//	if(!fs::has_known_protocol(key))
//	{
//		APPLOG_ERROR("Asset {0} has uknown protocol!", key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	auto cache_key = fs::replace(key, ":/data", ":/cache");
//	fs::path absolute_key = fs::absolute(fs::resolve_protocol(cache_key).string());

//	auto compiled_absolute_key = absolute_key.string() + ".asset";

//	fs::error_code err;
//	if(!fs::exists(compiled_absolute_key, err))
//	{
//		APPLOG_ERROR("Asset with key {0} and absolute_path {1} does not exist!", key, compiled_absolute_key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	struct wrapper_t
//	{
//		std::shared_ptr<::mesh> mesh = std::make_shared<::mesh>();
//	};

//	auto wrapper = std::make_shared<wrapper_t>();
//	auto read_memory_func = [wrapper, compiled_absolute_key]() mutable {
//		mesh::load_data data;
//		{
//			std::ifstream stream{compiled_absolute_key, std::ios::in | std::ios::binary};

//			if(stream.bad())
//			{
//				return false;
//			}

//			cereal::iarchive_binary_t ar(stream);

//			try_load(ar, cereal::make_nvp("mesh", data));
//		}
//		wrapper->mesh->prepare_mesh(data.vertex_format);
//		wrapper->mesh->set_vertex_source(&data.vertex_data[0], data.vertex_count, data.vertex_format);
//		wrapper->mesh->add_primitives(data.triangle_data);
//		wrapper->mesh->set_subset_count(data.material_count);
//		wrapper->mesh->bind_skin(data.skin_data);
//		wrapper->mesh->bind_armature(data.root_node);
//		wrapper->mesh->end_prepare(true, false, false, false);

//		return true;
//	};

//	auto create_resource_func = [ result = original, wrapper, key ](bool read_result) mutable
//	{
//		// Build the mesh
//		if(read_result)
//		{
//			wrapper->mesh->build_vb();
//			wrapper->mesh->build_ib();

//			if(wrapper->mesh->get_status() == mesh_status::prepared)
//			{
//				result.link->id = key;
//				result.link->asset = wrapper->mesh;
//			}
//			wrapper.reset();
//		}

//		return result;
//	};

//	auto ready_memory_task = ts.push_on_worker_thread(read_memory_func);
//	output = ts.push_on_owner_thread(create_resource_func, ready_memory_task);
//	return true;
//}

// template <>
// bool load_from_file<audio::sound>(core::task_future<asset_handle<audio::sound>>& output,
//								  const std::string& key)
//{
//	asset_handle<audio::sound> original;
//	if(output.is_ready())
//	{
//		original = output.get();
//	}

//	auto& ts = core::get_subsystem<core::task_system>();

//	auto create_resource_func_fallback = [ result = original, key ]() mutable
//	{
//		result.link->id = key;
//		return result;
//	};

//	if(!fs::has_known_protocol(key))
//	{
//		APPLOG_ERROR("Asset {0} has uknown protocol!", key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	auto cache_key = fs::replace(key, ":/data", ":/cache");
//	fs::path absolute_key = fs::absolute(fs::resolve_protocol(cache_key).string());

//	auto compiled_absolute_key = absolute_key.string() + ".asset";

//	fs::error_code err;
//	if(!fs::exists(compiled_absolute_key, err))
//	{
//		APPLOG_ERROR("Asset with key {0} and absolute_path {1} does not exist!", key, compiled_absolute_key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	struct wrapper_t
//	{
//		audio::sound_data data;
//	};

//	auto wrapper = std::make_shared<wrapper_t>();
//	auto read_memory_func = [wrapper, compiled_absolute_key]() mutable {
//		{
//			std::ifstream stream{compiled_absolute_key, std::ios::in | std::ios::binary};

//			if(stream.bad())
//			{
//				return false;
//			}

//			cereal::iarchive_binary_t ar(stream);

//			try_load(ar, cereal::make_nvp("sound", wrapper->data));
//		}
//		return true;
//	};

//	auto create_resource_func = [ result = original, wrapper, key ](bool read_result) mutable
//	{
//		if(read_result)
//		{
//			if(!wrapper->data.data.empty())
//			{
//				result.link->id = key;
//				result.link->asset = std::make_shared<audio::sound>(std::move(wrapper->data));
//			}
//			wrapper.reset();
//		}

//		return result;
//	};

//	auto ready_memory_task = ts.push_on_worker_thread(read_memory_func);
//	output = ts.push_on_owner_thread(create_resource_func, ready_memory_task);
//	return true;
//}

// template <>
// bool load_from_file<runtime::animation>(core::task_future<asset_handle<runtime::animation>>& output,
//										const std::string& key)
//{
//	asset_handle<runtime::animation> original;
//	if(output.is_ready())
//	{
//		original = output.get();
//	}

//	auto& ts = core::get_subsystem<core::task_system>();

//	auto create_resource_func_fallback = [ result = original, key ]() mutable
//	{
//		result.link->id = key;
//		return result;
//	};

//	if(!fs::has_known_protocol(key))
//	{
//		APPLOG_ERROR("Asset {0} has uknown protocol!", key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	auto cache_key = fs::replace(key, ":/data", ":/cache");
//	fs::path absolute_key = fs::absolute(fs::resolve_protocol(cache_key).string());

//	auto compiled_absolute_key = absolute_key.string() + ".asset";

//	fs::error_code err;
//	if(!fs::exists(compiled_absolute_key, err))
//	{
//		APPLOG_ERROR("Asset with key {0} and absolute_path {1} does not exist!", key, compiled_absolute_key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	struct wrapper_t
//	{
//		std::shared_ptr<runtime::animation> anim = std::make_shared<runtime::animation>();
//	};

//	auto wrapper = std::make_shared<wrapper_t>();
//	auto read_memory_func = [wrapper, compiled_absolute_key]() mutable {
//		auto& data = *wrapper->anim;
//		{
//			std::ifstream stream{compiled_absolute_key, std::ios::in | std::ios::binary};

//			if(stream.bad())
//			{
//				return false;
//			}

//			cereal::iarchive_binary_t ar(stream);

//			try_load(ar, cereal::make_nvp("animation", data));
//		}

//		return true;
//	};

//	auto create_resource_func = [ result = original, wrapper, key ](bool read_result) mutable
//	{
//		// Build the mesh
//		if(read_result && wrapper->anim)
//		{
//			result.link->id = key;
//			result.link->asset = wrapper->anim;

//			wrapper.reset();
//		};

//		return result;
//	};

//	auto ready_memory_task = ts.push_on_worker_thread(read_memory_func);
//	output = ts.push_on_owner_thread(create_resource_func, ready_memory_task);
//	return true;
//}

// template <>
// bool load_from_file<material>(core::task_future<asset_handle<material>>& output, const std::string& key)
//{
//	asset_handle<material> original;
//	if(output.is_ready())
//	{
//		original = output.get();
//	}

//	auto& ts = core::get_subsystem<core::task_system>();
//	auto& am = core::get_subsystem<asset_manager>();

//	auto create_resource_func_fallback = [ result = original, key ]() mutable
//	{
//		result.link->id = key;
//		return result;
//	};

//	if(!fs::has_known_protocol(key))
//	{
//		APPLOG_ERROR("Asset {0} has uknown protocol!", key);
//		output = am.load<material>("embedded:/fallback");

//		return true;
//	}

//	auto cache_key = fs::replace(key, ":/data", ":/cache");
//	fs::path absolute_key = fs::absolute(fs::resolve_protocol(cache_key).string());

//	auto compiled_absolute_key = absolute_key.string() + ".asset";

//	fs::error_code err;
//	if(!fs::exists(compiled_absolute_key, err))
//	{
//		APPLOG_ERROR("Asset with key {0} and absolute_path {1} does not exist!", key, compiled_absolute_key);
//		output = am.load<material>("embedded:/fallback");
//		return true;
//	}

//	struct wrapper_t
//	{
//		std::shared_ptr<::material> material = std::make_shared<::material>();
//	};

//	auto wrapper = std::make_shared<wrapper_t>();

//	auto read_memory_func = [wrapper, compiled_absolute_key]() mutable {
//		std::ifstream stream{compiled_absolute_key, std::ios::in | std::ios::binary};

//		if(stream.bad())
//		{
//			return false;
//		}
//		cereal::iarchive_binary_t ar(stream);

//		try_load(ar, cereal::make_nvp("material", wrapper->material));

//		return true;
//	};

//	auto create_resource_func = [ result = original, wrapper, key ](bool read_result) mutable
//	{
//		if(read_result)
//		{
//			result.link->id = key;
//			result.link->asset = wrapper->material;
//			wrapper.reset();
//		}

//		return result;
//	};

//	auto ready_memory_task = ts.push_on_worker_thread(read_memory_func);
//	output = ts.push_on_owner_thread(create_resource_func, ready_memory_task);
//	return true;
//}

// template <>
// bool load_from_file<prefab>(core::task_future<asset_handle<prefab>>& output, const std::string& key)
//{
//	asset_handle<prefab> original;
//	if(output.is_ready())
//	{
//		original = output.get();
//	}

//	auto& ts = core::get_subsystem<core::task_system>();

//	auto create_resource_func_fallback = [ result = original, key ]() mutable
//	{
//		result.link->id = key;
//		return result;
//	};

//	if(!fs::has_known_protocol(key))
//	{
//		APPLOG_ERROR("Asset {0} has uknown protocol!", key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	auto cache_key = fs::replace(key, ":/data", ":/cache");
//	fs::path absolute_key = fs::absolute(fs::resolve_protocol(cache_key).string());

//	auto compiled_absolute_key = absolute_key.string() + ".asset";

//	fs::error_code err;
//	if(!fs::exists(compiled_absolute_key, err))
//	{
//		APPLOG_ERROR("Asset with key {0} and absolute_path {1} does not exist!", key, compiled_absolute_key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	std::shared_ptr<std::istringstream> read_memory = std::make_shared<std::istringstream>();

//	auto read_memory_func = [read_memory, compiled_absolute_key]() {
//		if(!read_memory)
//		{
//			return false;
//		}

//		auto stream =
//			std::fstream{compiled_absolute_key, std::fstream::in | std::fstream::out | std::ios::binary};
//		auto mem = fs::read_stream(stream);
//		*read_memory = std::istringstream(std::string(mem.begin(), mem.end()));

//		return true;
//	};

//	auto create_resource_func = [ result = original, read_memory, key ](bool read_result) mutable
//	{
//		if(read_result)
//		{
//			auto pfab = std::make_shared<prefab>();
//			pfab->data = read_memory;

//			result.link->id = key;
//			result.link->asset = pfab;
//		}

//		return result;
//	};

//	auto ready_memory_task = ts.push_on_worker_thread(read_memory_func);
//	output = ts.push_on_owner_thread(create_resource_func, ready_memory_task);
//	return true;
//}

// template <>
// bool load_from_file<scene>(core::task_future<asset_handle<scene>>& output, const std::string& key)
//{
//	asset_handle<scene> original;
//	if(output.is_ready())
//	{
//		original = output.get();
//	}

//	auto& ts = core::get_subsystem<core::task_system>();

//	auto create_resource_func_fallback = [ result = original, key ]() mutable
//	{
//		result.link->id = key;
//		return result;
//	};

//	if(!fs::has_known_protocol(key))
//	{
//		APPLOG_ERROR("Asset {0} has uknown protocol!", key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	auto cache_key = fs::replace(key, ":/data", ":/cache");
//	fs::path absolute_key = fs::absolute(fs::resolve_protocol(cache_key).string());

//	auto compiled_absolute_key = absolute_key.string() + ".asset";

//	fs::error_code err;
//	if(!fs::exists(compiled_absolute_key, err))
//	{
//		APPLOG_ERROR("Asset with key {0} and absolute_path {1} does not exist!", key, compiled_absolute_key);
//		output = ts.push_or_execute_on_worker_thread(create_resource_func_fallback);
//		return true;
//	}

//	std::shared_ptr<std::istringstream> read_memory = std::make_shared<std::istringstream>();

//	auto read_memory_func = [read_memory, compiled_absolute_key]() {
//		if(!read_memory)
//		{
//			return false;
//		}

//		auto stream =
//			std::fstream{compiled_absolute_key, std::fstream::in | std::fstream::out | std::ios::binary};
//		auto mem = fs::read_stream(stream);
//		*read_memory = std::istringstream(std::string(mem.begin(), mem.end()));

//		return true;
//	};

//	auto create_resource_func = [ result = original, read_memory, key ](bool read_result) mutable
//	{
//		if(read_result)
//		{
//			auto sc = std::make_shared<scene>();
//			sc->data = read_memory;

//			result.link->id = key;
//			result.link->asset = sc;
//		}

//		return result;
//	};

//	auto ready_memory_task = ts.push_on_worker_thread(read_memory_func);
//	output = ts.push_on_owner_thread(create_resource_func, ready_memory_task);
//	return true;
//}
} // namespace ace::asset_reader
