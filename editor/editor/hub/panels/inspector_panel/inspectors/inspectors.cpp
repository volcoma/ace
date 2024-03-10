#include "inspectors.h"
#include "editor/imgui/integration/fonts/icons/icons_material_design_icons.h"
#include "imgui/imgui.h"
#include <unordered_map>
#include <vector>

namespace ace
{

inspector_registry::inspector_registry()
{
    auto inspector_types = rttr::type::get<inspector>().get_derived_classes();
    for(auto& inspector_type : inspector_types)
    {
        auto inspected_type_var = inspector_type.get_metadata("inspected_type");
        if(inspected_type_var)
        {
            auto inspected_type = inspected_type_var.get_value<rttr::type>();
            auto inspector_var = inspector_type.create();
            if(inspector_var)
            {
                type_map[inspected_type] = inspector_var.get_value<std::shared_ptr<inspector>>();
            }
        }
    }
}

auto get_inspector(rtti::context& ctx, rttr::type type) -> std::shared_ptr<inspector>
{
    auto& registry = ctx.get<inspector_registry>();
    return registry.type_map[type];
}

auto inspect_property(rtti::context& ctx, rttr::instance& object, const rttr::property& prop) -> bool
{
    bool prop_changed = false;
    auto prop_var = prop.get_value(object);
    bool is_readonly = prop.is_readonly();
    bool is_array = prop_var.is_sequential_container();
    bool is_associative_container = prop_var.is_associative_container();
    bool is_enum = prop.is_enumeration();
    rttr::instance prop_object = prop_var;
    auto prop_type = prop_object.get_derived_type();
    auto prop_inspector = get_inspector(ctx, prop_type);

    var_info info;
    info.read_only = is_readonly;
    info.is_property = true;

    if(prop_inspector)
    {
        prop_inspector->before_inspect(prop);
    }

    {
        auto get_meta = [&prop](const rttr::variant& name) -> rttr::variant
        {
            return prop.get_metadata(name);
        };
        if(is_array)
        {
            prop_changed |= inspect_array(ctx, prop_var, prop, info, get_meta);
        }
        else if(is_associative_container)
        {
            prop_changed |= inspect_associative_container(ctx, prop_var, prop, info);
        }
        else if(is_enum)
        {
            auto enumeration = prop.get_enumeration();
            property_layout layout(prop);
            prop_changed |= inspect_enum(ctx, prop_var, enumeration, info);
        }
        else
        {
            prop_changed |= inspect_var(ctx, prop_var, info, get_meta);
        }
    }

    if(prop_changed && !is_readonly)
    {
        prop.set_value(object, prop_var);
    }

    if(prop_inspector)
    {
        prop_inspector->after_inspect(prop);
    }

    return prop_changed;
}

auto inspect_var(rtti::context& ctx,
                 rttr::variant& var,
                 const var_info& info,
                 const inspector::meta_getter& get_metadata) -> bool
{
    rttr::instance object = var;
    auto type = object.get_derived_type();
    auto properties = type.get_properties();

    bool changed = false;

    auto inspector = get_inspector(ctx, type);
    if(inspector)
    {
        changed |= inspector->inspect(ctx, var, info, get_metadata);
    }
    else
    {
        changed |= inspect_var_properties(ctx, var, info, get_metadata);
    }

    return changed;
}

auto inspect_var_properties(rtti::context& ctx,
                            rttr::variant& var,
                            const var_info& info,
                            const inspector::meta_getter& get_metadata) -> bool
{
    rttr::instance object = var;
    auto type = object.get_derived_type();
    auto properties = type.get_properties();

    bool changed = false;

    if(properties.empty())
    {
        if(type.is_enumeration())
        {
            auto enumeration = type.get_enumeration();
            changed |= inspect_enum(ctx, var, enumeration, info);
        }
    }
    else
    {
        for(auto& prop : properties)
        {
            changed |= inspect_property(ctx, object, prop);
        }
    }

    return changed;
}

auto inspect_array(rtti::context& ctx,
                   rttr::variant& var,
                   const rttr::property& prop,
                   const var_info& info,
                   const inspector::meta_getter& get_metadata) -> bool
{
    auto view = var.create_sequential_view();
    auto size = view.get_size();
    bool changed = false;
    auto int_size = static_cast<int>(size);

    property_layout layout;
    layout.set_data(prop);

    bool open = true;
    if(view.is_dynamic())
    {
        open = layout.push_tree_layout();
        if(!info.read_only)
        {
            if(ImGui::InputInt("", &int_size))
            {
                if(int_size < 0)
                    int_size = 0;
                size = static_cast<std::size_t>(int_size);
                changed |= view.set_size(size);
            }
        }
        else
        {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(std::to_string(int_size).c_str());
        }
    }

    if(open)
    {
        layout.pop_layout();

        int index_to_remove = -1;
        for(std::size_t i = 0; i < size; ++i)
        {
            auto value = view.get_value(i).extract_wrapped_value();
            std::string element = "Element ";
            element += std::to_string(i);

            ImGui::Separator();

            // ImGui::SameLine();
            auto pos_before = ImGui::GetCursorPos();
            {
                property_layout layout;
                layout.set_data(element.data(), {}, true);
                layout.push_tree_layout(ImGuiTreeNodeFlags_Leaf);

                changed |= inspect_var(ctx, value, info, get_metadata);
            }
            auto pos_after = ImGui::GetCursorPos();

            if(changed)
                view.set_value(i, value);

            ImGui::SetCursorPos(pos_before);

            ImGui::PushID(i);
            ImGui::AlignTextToFramePadding();
            if(ImGui::Button(ICON_MDI_DELETE, ImVec2(0, ImGui::GetItemRectSize().y)))
            {
                index_to_remove = i;
            }
            ImGui::PopID();
            ImGui::SetCursorPos(pos_after);
        }

        if(index_to_remove != -1)
        {
            view.erase(view.begin() + index_to_remove);
            changed = true;
        }
    }

    return changed;
}

auto inspect_associative_container(rtti::context& ctx,
                                   rttr::variant& var,
                                   const rttr::property& prop,
                                   const var_info& info) -> bool
{
    auto associative_view = var.create_associative_view();
    // auto size = associative_view.get_size();
    bool changed = false;

    return changed;
}

auto inspect_enum(rtti::context& ctx, rttr::variant& var, rttr::enumeration& data, const var_info& info) -> bool
{
    auto strings = data.get_names();
    std::vector<const char*> cstrings{};
    cstrings.reserve(strings.size());

    for(const auto& string : strings)
        cstrings.push_back(string.data());

    if(info.read_only)
    {
        int listbox_item_current = var.to_int();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(cstrings[std::size_t(listbox_item_current)]);
    }
    else
    {
        int listbox_item_current = var.to_int();
        int listbox_item_size = static_cast<int>(cstrings.size());
        if(ImGui::Combo("##enum", &listbox_item_current, cstrings.data(), listbox_item_size, listbox_item_size))
        {
            rttr::variant arg(listbox_item_current);
            arg.convert(var.get_type());
            var = arg;
            return true;
        }
    }

    return false;
}

auto get_meta_empty(const rttr::variant& other) -> rttr::variant
{
    return rttr::variant();
}

} // namespace ace
