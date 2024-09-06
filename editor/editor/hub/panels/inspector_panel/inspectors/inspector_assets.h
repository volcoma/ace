#pragma once

#include "inspector.h"
#include <engine/assets/asset_handle.h>

namespace gfx
{
struct texture;
}

namespace ace
{
class mesh;
class material;
struct physics_material;
struct animation_clip;
struct audio_clip;
struct prefab;
struct scene_prefab;

struct inspector_asset_handle : public inspector
{
    REFLECTABLEV(inspector_asset_handle, inspector)

    ImGuiTextFilter filter;
};

struct inspector_asset_handle_texture : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_texture, inspector_asset_handle)

    void draw_image(const asset_handle<gfx::texture>& data, ImVec2 size) const;
    auto inspect_as_property(rtti::context& ctx, asset_handle<gfx::texture>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_texture, asset_handle<gfx::texture>)

struct inspector_asset_handle_material : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_material, inspector_asset_handle)
    auto inspect_as_property(rtti::context& ctx, asset_handle<material>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_material, asset_handle<material>)

struct inspector_asset_handle_mesh : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_mesh, inspector_asset_handle)
    auto inspect_as_property(rtti::context& ctx, asset_handle<mesh>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_mesh, asset_handle<mesh>)

struct inspector_asset_handle_animation : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_animation, inspector_asset_handle)
    auto inspect_as_property(rtti::context& ctx, asset_handle<animation_clip>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_animation, asset_handle<animation_clip>)

struct inspector_asset_handle_prefab : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_prefab, inspector_asset_handle)
    auto inspect_as_property(rtti::context& ctx, asset_handle<prefab>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_prefab, asset_handle<prefab>)

struct inspector_asset_handle_scene_prefab : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_scene_prefab, inspector_asset_handle)
    auto inspect_as_property(rtti::context& ctx, asset_handle<scene_prefab>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_scene_prefab, asset_handle<scene_prefab>)

struct inspector_asset_handle_physics_material : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_physics_material, inspector_asset_handle)
    auto inspect_as_property(rtti::context& ctx, asset_handle<physics_material>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_physics_material, asset_handle<physics_material>)

struct inspector_asset_handle_audio_clip : public inspector_asset_handle
{
    REFLECTABLEV(inspector_asset_handle_audio_clip, inspector_asset_handle)
    auto inspect_as_property(rtti::context& ctx, asset_handle<audio_clip>& data) -> inspect_result;
    auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result;
};
REFLECT_INSPECTOR_INLINE(inspector_asset_handle_audio_clip, asset_handle<audio_clip>)

} // namespace ace
