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
struct animation;

struct inspector_asset_handle : public inspector
{
    REFLECTABLEV(inspector_asset_handle, inspector)

    ImGuiTextFilter filter;
};

struct inspector_asset_handle_texture : public inspector_asset_handle
{
	REFLECTABLEV(inspector_asset_handle_texture, inspector_asset_handle)

    void draw_image(const asset_handle<gfx::texture>& data, ImVec2 size) const;
    bool inspect_as_property(rtti::context& ctx, asset_handle<gfx::texture>& data);
	bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);

};
INSPECTOR_REFLECT(inspector_asset_handle_texture, asset_handle<gfx::texture>)

struct inspector_asset_handle_material : public inspector_asset_handle
{
	REFLECTABLEV(inspector_asset_handle_material, inspector_asset_handle)
    bool inspect_as_property(rtti::context& ctx, asset_handle<material>& data);

	bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
INSPECTOR_REFLECT(inspector_asset_handle_material, asset_handle<material>)

struct inspector_asset_handle_mesh : public inspector_asset_handle
{
	REFLECTABLEV(inspector_asset_handle_mesh, inspector_asset_handle)
    bool inspect_as_property(rtti::context& ctx, asset_handle<mesh>& data);

	bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
INSPECTOR_REFLECT(inspector_asset_handle_mesh, asset_handle<mesh>)

struct inspector_asset_handle_animation : public inspector_asset_handle
{
	REFLECTABLEV(inspector_asset_handle_animation, inspector_asset_handle)
    bool inspect_as_property(rtti::context& ctx, asset_handle<animation>& data);

	bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
INSPECTOR_REFLECT(inspector_asset_handle_animation, asset_handle<animation>)


}
