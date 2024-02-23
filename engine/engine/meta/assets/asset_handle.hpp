#pragma once

#include <engine/assets/asset_handle.h>
#include <engine/assets/asset_manager.h>
#include <engine/engine.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace cereal
{

template <typename Archive, typename T>
inline void SAVE_FUNCTION_NAME(Archive& ar, asset_handle<T> const& obj)
{
	try_save(ar, cereal::make_nvp("id", obj.id()));
}

template <typename Archive, typename T>
inline void LOAD_FUNCTION_NAME(Archive& ar, asset_handle<T>& obj)
{
    std::string id;
	try_load(ar, cereal::make_nvp("id", id));

	if(id.empty())
	{
		obj = asset_handle<T>();
	}
	else
	{
        auto& ctx = ace::engine::context();
        auto& am = ctx.get<ace::asset_manager>();
        obj = am.load<T>(id);
	}
}
}
