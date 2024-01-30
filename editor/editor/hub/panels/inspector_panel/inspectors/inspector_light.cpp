#include "inspector_light.h"
#include "inspectors.h"

namespace ace
{
bool inspector_light_component::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    auto& data = *var.get_value<light_component*>();
	auto light_val = data.get_light();
    bool changed = ::ace::inspect(ctx, light_val);

	if(light_val.type == light_type::spot)
	{
        changed |= ::ace::inspect(ctx, light_val.spot_data);
	}
	else if(light_val.type == light_type::point)
	{
		changed |= ::ace::inspect(ctx, light_val.point_data);
	}
	else if(light_val.type == light_type::directional)
	{
        changed |= ::ace::inspect(ctx, light_val.directional_data);
	}

	if(changed)
	{
		data.set_light(light_val);
		return true;
	}

	return false;
}

bool inspector_skylight_component::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    auto& data = *var.get_value<light_component*>();

	return false;
}

bool inspector_reflection_probe_component::inspect(rtti::context& ctx,
                                         rttr::variant& var,
                                         const var_info& info,
                                         const meta_getter& get_metadata)
{
    auto& data = *var.get_value<reflection_probe_component*>();
    auto probe = data.get_probe();

    bool changed = ::ace::inspect(ctx, probe);

    if(probe.type == probe_type::box)
    {
        changed |= ::ace::inspect(ctx, probe.box_data);
    }
    else if(probe.type == probe_type::sphere)
    {
        changed |= ::ace::inspect(ctx, probe.sphere_data);
    }

    if(changed)
    {
        data.set_probe(probe);
        return true;
    }

    return false;
}
} // namespace ace
