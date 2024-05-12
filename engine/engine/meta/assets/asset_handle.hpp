#pragma once

#include <engine/assets/asset_handle.h>
#include <engine/assets/asset_manager.h>
#include <engine/engine.h>
#include <engine/meta/core/common/basetypes.hpp>

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace cereal
{

template<typename Archive, typename T>
inline void SAVE_FUNCTION_NAME(Archive& ar, asset_handle<T> const& obj)
{
    //if(!obj.uid().is_nil())
    {
        try_save(ar, cereal::make_nvp("uid", obj.uid()));
    }

}

template<typename Archive, typename T>
inline void LOAD_FUNCTION_NAME(Archive& ar, asset_handle<T>& obj)
{
    hpp::uuid uid{};
    try_load(ar, cereal::make_nvp("uid", uid));

    if(uid.is_nil())
    {
        obj = {};
    }
    else
    {
        auto& ctx = ace::engine::context();
        auto& am = ctx.get<ace::asset_manager>();
        obj = am.get_asset<T>(uid);
    }
}
} // namespace cereal
