#include "inspectors.h"
#include "editor/imgui/integration/fonts/icons/icons_material_design_icons.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
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

auto get_inspector(rtti::context& ctx, const rttr::type& type) -> std::shared_ptr<inspector>
{
    auto& registry = ctx.get<inspector_registry>();
    return registry.type_map[type];
}

auto inspect_property(rtti::context& ctx, rttr::instance& object, const rttr::property& prop) -> inspect_result
{
    inspect_result result{};
    auto prop_var = prop.get_value(object);
    bool is_readonly = prop.is_readonly();
    bool is_array = prop_var.is_sequential_container();
    bool is_associative_container = prop_var.is_associative_container();
    bool is_enum = prop.is_enumeration();
    rttr::instance prop_object = prop_var;
    auto prop_type = prop_object.get_derived_type();
    auto prop_inspector = get_inspector(ctx, prop_type);

    is_readonly |= ImGui::IsReadonly();

    var_info info;
    info.read_only = is_readonly;
    info.is_property = true;

    if(prop_inspector)
    {
        prop_inspector->before_inspect(prop);
    }

    ImGui::PushReadonly(is_readonly);

    {
        auto get_meta = [&prop](const rttr::variant& name) -> rttr::variant
        {
            return prop.get_metadata(name);
        };
        if(is_array)
        {
            result |= inspect_array(ctx, prop_var, prop, info, get_meta);
        }
        else if(is_associative_container)
        {
            result |= inspect_associative_container(ctx, prop_var, prop, info, get_meta);
        }
        else if(is_enum)
        {
            auto enumeration = prop.get_enumeration();
            property_layout layout(prop);
            result |= inspect_enum(ctx, prop_var, enumeration, info);
        }
        else
        {
            result |= inspect_var(ctx, prop_var, info, get_meta);
        }
    }

    if(result.changed && !is_readonly)
    {
        prop.set_value(object, prop_var);
    }

    // ImGui::PopEnabled();
    ImGui::PopReadonly();
    if(prop_inspector)
    {
        prop_inspector->after_inspect(prop);
    }

    return result;
}

auto inspect_var(rtti::context& ctx,
                 rttr::variant& var,
                 const var_info& info,
                 const inspector::meta_getter& get_metadata) -> inspect_result
{
    rttr::instance object = var;
    auto type = object.get_derived_type();
    auto properties = type.get_properties();

    inspect_result result{};

    auto inspector = get_inspector(ctx, type);
    if(inspector)
    {
        result |= inspector->inspect(ctx, var, info, get_metadata);
    }
    else
    {
        result |= inspect_var_properties(ctx, var, info, get_metadata);
    }

    return result;
}

auto inspect_var_properties(rtti::context& ctx,
                            rttr::variant& var,
                            const var_info& info,
                            const inspector::meta_getter& get_metadata) -> inspect_result
{
    rttr::instance object = var;
    auto type = object.get_derived_type();
    auto properties = type.get_properties();

    inspect_result result{};
    if(properties.empty())
    {
        if(type.is_enumeration())
        {
            auto enumeration = type.get_enumeration();
            result |= inspect_enum(ctx, var, enumeration, info);
        }
    }
    else
    {
        for(auto& prop : properties)
        {
            result |= inspect_property(ctx, object, prop);
        }
    }

    return result;
}

auto inspect_array(rtti::context& ctx,
                   rttr::variant& var,
                   const rttr::property& prop,
                   const var_info& info,
                   const inspector::meta_getter& get_metadata) -> inspect_result
{
    auto view = var.create_sequential_view();
    auto size = view.get_size();
    inspect_result result{};
    auto int_size = static_cast<int>(size);

    ImGui::BeginGroup();
    property_layout layout;
    layout.set_data(prop);

    bool open = true;
    if(view.is_dynamic())
    {
        open = layout.push_tree_layout();
        {
            ImGuiInputTextFlags flags = 0;

            if(info.read_only)
            {
                flags |= ImGuiInputTextFlags_ReadOnly;
            }

            if(ImGui::InputInt("##array", &int_size, 1, 100, flags))
            {
                if(int_size < 0)
                    int_size = 0;
                size = static_cast<std::size_t>(int_size);
                result.changed |= view.set_size(size);
                result.edit_finished = true;
            }

            ImGui::DrawItemActivityOutline();
        }
    }

    if(open)
    {
        layout.pop_layout();

        ImGui::TreePush("test");


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
                layout.set_data(element, {}, true);
                layout.push_tree_layout(ImGuiTreeNodeFlags_Leaf);

                result |= inspect_var(ctx, value, info, get_metadata);
            }
            auto pos_after = ImGui::GetCursorPos();

            if(result.changed)
                view.set_value(i, value);

            if(!info.read_only)
            {
                ImGui::SetCursorPos(pos_before);

                ImGui::PushID(i);
                ImGui::AlignTextToFramePadding();
                if(ImGui::Button(ICON_MDI_DELETE, ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing())))
                {
                    index_to_remove = i;
                }
                ImGui::SetItemTooltip("Remove element.");
                ImGui::PopID();
                ImGui::SetCursorPos(pos_after);
                ImGui::Dummy({});
            }
        }

        if(index_to_remove != -1)
        {
            view.erase(view.begin() + index_to_remove);
            result.changed = true;
            result.edit_finished = true;
        }

        ImGui::TreePop();

    }
    ImGui::EndGroup();
    ImGui::RenderFrameEx(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

    return result;
}

auto inspect_associative_container(rtti::context& ctx,
                                   rttr::variant& var,
                                   const rttr::property& prop,
                                   const var_info& info,
                                   const inspector::meta_getter& get_metadata) -> inspect_result
{
    auto view = var.create_associative_view();
    auto size = view.get_size();
    auto int_size = static_cast<int>(size);

    inspect_result result{};

    // property_layout layout;
    // layout.set_data(prop);

    // bool open = true;
    // {
    //     open = layout.push_tree_layout();
    //     {
    //         ImGuiInputTextFlags flags = 0;

    //         if(info.read_only)
    //         {
    //             flags |= ImGuiInputTextFlags_ReadOnly;
    //         }

    //         if(ImGui::InputInt("##assoc", &int_size, 1, 100, flags))
    //         {
    //             if(int_size < 0)
    //                 int_size = 0;
    //             size = static_cast<std::size_t>(int_size);
    //             result.changed |= view.insert(view.get_key_type().create()).second;
    //             result.edit_finished = true;
    //         }

    //         ImGui::DrawItemActivityOutline();
    //     }
    // }

    // if(open)
    // {
    //     layout.pop_layout();

    //     int i = 0;
    //     int index_to_remove = -1;
    //     rttr::argument key_to_remove{};
    //     for(const auto& item : view)
    //     {
    //         auto key = item.first.extract_wrapped_value();
    //         auto value = item.second.extract_wrapped_value();

    //         ImGui::Separator();

    //         // ImGui::SameLine();
    //         auto pos_before = ImGui::GetCursorPos();
    //         {
    //             property_layout layout;
    //             layout.set_data(key.to_string(), {}, true);
    //             layout.push_tree_layout(ImGuiTreeNodeFlags_Leaf);

    //             result |= inspect_var(ctx, value, info, get_metadata);
    //         }
    //         auto pos_after = ImGui::GetCursorPos();

    //         // if(result.changed)
    //         //     view.set_value(i, value);

    //         if(!info.read_only)
    //         {
    //             ImGui::SetCursorPos(pos_before);

    //             ImGui::PushID(i);
    //             ImGui::AlignTextToFramePadding();
    //             if(ImGui::Button(ICON_MDI_DELETE, ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing())))
    //             {
    //                 key_to_remove = key;
    //                 index_to_remove = i;
    //             }
    //             ImGui::SetItemTooltip("Remove element.");
    //             ImGui::PopID();
    //             ImGui::SetCursorPos(pos_after);
    //             ImGui::Dummy({});

    //         }

    //         i++;
    //     }

    //     if(index_to_remove != -1)
    //     {
    //         view.erase(key_to_remove);
    //         result.changed = true;
    //         result.edit_finished = true;
    //     }
    // }

    return result;
}

auto inspect_enum(rtti::context& ctx, rttr::variant& var, rttr::enumeration& data, const var_info& info)
    -> inspect_result
{
    auto current_name = data.value_to_name(var);

    auto strings = data.get_names();
    std::vector<const char*> cstrings{};
    cstrings.reserve(strings.size());

    int current_idx = 0;
    int i = 0;
    for(const auto& string : strings)
    {
        cstrings.push_back(string.data());

        if(current_name == string)
        {
            current_idx = i;
        }
        i++;
    }

    inspect_result result{};

    if(info.read_only)
    {
        ImGui::LabelText("##enum", "%s", cstrings[current_idx]);
    }
    else
    {
        int listbox_item_size = static_cast<int>(cstrings.size());

        ImGuiComboFlags flags = 0;


        if(ImGui::BeginCombo("##enum", cstrings[current_idx], flags))
        {
            for(int n = 0; n < listbox_item_size; n++)
            {
                const bool is_selected = (current_idx == n);

                if(ImGui::Selectable(cstrings[n], is_selected))
                {
                    current_idx = n;
                    result.changed = true;
                    result.edit_finished |= true;
                    var = data.name_to_value(cstrings[current_idx]);
                }

                ImGui::DrawItemActivityOutline();


                if(is_selected)
                    ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }

        // if(ImGui::Combo("##enum", &current_idx, cstrings.data(), listbox_item_size, listbox_item_size))
        // {
        //     var = data.name_to_value(cstrings[current_idx]);
        //     result.changed |= true;
        //     result.edit_finished |= true;
        // }

        ImGui::DrawItemActivityOutline();
    }

    return result;
}

auto get_meta_empty(const rttr::variant& other) -> rttr::variant
{
    return rttr::variant();
}

} // namespace ace
