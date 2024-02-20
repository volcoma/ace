#include "inspector_physics_shape.h"
#include "imgui/imgui.h"
#include "inspectors.h"

namespace ace
{
bool inspector_physics_compound_shape::inspect(rtti::context& ctx,
                              rttr::variant& var,
                              const var_info& info,
                              const meta_getter& get_metadata)
{
    auto& data = var.get_value<physics_compound_shape>();


    bool changed = false;

    if(std::holds_alternative<physics_box_shape>(data.shape))
    {
        auto& shape = std::get<physics_box_shape>(data.shape);
        changed = ::ace::inspect(ctx, shape);
    }
    else
    {

        ImGui::LabelText("Unknown", "%s", "test");
    }


	if(changed)
	{
//        data.shapes = shapes;
//		data.set_shape(data);
		return true;
	}

	return false;
}

} // namespace ace
