#include "inspector_assets.h"
#include "inspectors.h"

#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_writer.h>
#include <engine/rendering/material.h>

#include <editor/assets/asset_extensions.h>
#include <editor/editing/editing_system.h>

#include <filesystem/filesystem.h>
#include <graphics/texture.h>
#include <imgui/imgui_internal.h>

namespace ace
{
template<typename asset_t>
static bool process_drag_drop_target(ace::asset_manager& am, asset_handle<asset_t>& entry)
{
    if(ImGui::BeginDragDropTarget())
    {
        if(ImGui::IsDragDropPayloadBeingAccepted())
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        else
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
        }

        for(const auto& type : ex::get_suported_formats<asset_t>())
        {
            auto payload = ImGui::AcceptDragDropPayload(type.c_str());
            if(payload)
            {
                std::string absolute_path(reinterpret_cast<const char*>(payload->Data), std::size_t(payload->DataSize));

                std::string key = fs::convert_to_protocol(fs::path(absolute_path)).generic_string();
                auto entry_future = am.template find_asset_entry<asset_t>(key);
                if(entry_future.is_ready())
                {
                    entry = entry_future;
                }

                if(entry.is_valid())
                {
                    return true;
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
    return false;
    ;
}

void inspector_asset_handle_texture::draw_image(const asset_handle<gfx::texture>& data, ImVec2 size) const
{
    if(data.is_ready())
    {
        auto sz = ImGui::GetSize(data, size);
        ImGui::ImageWithAspect(ImGui::ToId(data), sz, size);
    }
    else
    {
        ImGui::Dummy(size);
        ImGui::RenderFrameBorder(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }
}

bool inspector_asset_handle_texture::inspect_as_property(rtti::context& ctx, asset_handle<gfx::texture>& data)
{
    bool changed = false;
    auto& am = ctx.get<ace::asset_manager>();

    float available = math::min(64.0f, ImGui::GetContentRegionAvail().x / 1.5f);

    ImGui::BeginGroup();
    {
        draw_image(data, ImVec2(available, available));

        ImGui::SameLine();
        ImGui::BeginGroup();
        {
            if(ImGui::Button("Clear"))
            {
                data = asset_handle<gfx::texture>();
                changed = true;
            }
            if(data && ImGui::Button("Select"))
            {
                auto& es = ctx.get<editing_system>();
                es.select(data);
            }
        }
        ImGui::EndGroup();
        std::string item = !data.id().empty() ? data.id() : "none";
        rttr::variant var_str = item;
        if(inspect_var(ctx, var_str))
        {
            item = var_str.to_string();
            if(item.empty())
            {
                data = {};
            }
            else
            {
                data = am.load<gfx::texture>(item);
            }

            changed = true;
        }
    }
    ImGui::EndGroup();

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_texture::inspect(rtti::context& ctx,
                                             rttr::variant& var,
                                             const var_info& info,
                                             const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<gfx::texture>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    auto& am = ctx.get<ace::asset_manager>();
    bool changed = false;

    float available = ImGui::GetContentRegionAvail().x;

    if(ImGui::BeginTabBar("asset_handle_texture",
                          ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll))
    {
        if(ImGui::BeginTabItem("Info"))
        {
            draw_image(data, ImVec2(available, available));

            if(data.is_ready())
            {
                rttr::variant vari = data.get().info;
                changed |= inspect_var(ctx, vari);
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Import"))
        {
            ImGui::TextUnformatted("Import options");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    return changed;
}

bool inspector_asset_handle_material::inspect_as_property(rtti::context& ctx, asset_handle<material>& data)
{
    auto& am = ctx.get<asset_manager>();

    bool changed = false;
    std::string item = !data.id().empty() ? data.id() : "none";
    rttr::variant var_str = item;
    if(inspect_var(ctx, var_str))
    {
        item = var_str.to_string();
        if(item.empty())
        {
            data = {};
        }
        else
        {
            data = am.load<material>(item);
        }

        changed = true;
    }

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_material::inspect(rtti::context& ctx,
                                              rttr::variant& var,
                                              const var_info& info,
                                              const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<material>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    auto& am = ctx.get<asset_manager>();
    bool changed = false;

    if(ImGui::Button("SAVE CHANGES##top", ImVec2(-1, 0)))
    {
        asset_writer::save_to_file(data.id(), data);
    }
    ImGui::Separator();
    {
        rttr::variant vari = &data.get();
        changed |= inspect_var(ctx, vari);
    }
    ImGui::Separator();
    if(ImGui::Button("SAVE CHANGES##bottom", ImVec2(-1, 0)))
    {
        asset_writer::save_to_file(data.id(), data);
    }
    return changed;
}

} // namespace ace
