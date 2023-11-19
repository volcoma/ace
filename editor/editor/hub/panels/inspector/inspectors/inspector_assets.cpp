#include "inspector_assets.h"
#include "inspectors.h"

#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_writer.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>

#include <editor/assets/asset_extensions.h>
#include <editor/editing/thumbnail_system.h>

#include <filesystem/filesystem.h>
#include <graphics/texture.h>
#include <imgui/imgui_internal.h>

namespace ace
{
namespace
{
template<typename T>
bool process_drag_drop_target(asset_manager& am, asset_handle<T>& entry)
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

        for(const auto& type : ex::get_suported_formats<T>())
        {
            auto payload = ImGui::AcceptDragDropPayload(type.c_str());
            if(payload)
            {
                std::string absolute_path(reinterpret_cast<const char*>(payload->Data), std::size_t(payload->DataSize));

                std::string key = fs::convert_to_protocol(fs::path(absolute_path)).generic_string();
                auto entry_future = am.template find_asset_entry<T>(key);
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

template<typename T>
bool pick_asset(thumbnail_system& ths, asset_manager& am, asset_handle<T>& data, const std::string& type)
{
    if(data)
    {
        const auto& thumbnail = ths.get_thumbnail(data);

        auto fh = ImGui::GetFrameHeight();
        ImVec2 item_size = ImVec2(fh, fh);
        ImVec2 texture_size = ImGui::GetSize(thumbnail, item_size);

//        ImGui::AlignTextToFramePadding();
        ImGui::ImageButtonWithAspectAndTextBelow(ImGui::ToId(thumbnail), "", texture_size, item_size);

        ImGui::SameLine();
    }

    std::string item = data ? data.name() : fmt::format("None ({})", type);
    ImGui::AlignTextToFramePadding();
    if(ImGui::Selectable(item.c_str()))
    {
        ImGui::OpenPopup("Pick Asset");
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.4f);
    }

    bool changed = false;
    if(ImGui::BeginPopupModal("Pick Asset"))
    {
        auto assets = am.get_assets<T>();

        const float size = 100.0f;
        ImGui::ItemBrowser(size,
                           assets.size(),
                           [&](int index)
                           {
                               auto& asset = assets[index];
                               const auto& thumbnail = ths.get_thumbnail(asset);

                               ImVec2 item_size = {size, size};
                               ImVec2 texture_size = ImGui::GetSize(thumbnail, item_size);
                               if(ImGui::ImageButtonWithAspectAndTextBelow(ImGui::ToId(thumbnail),
                                                                           asset.name(),
                                                                           texture_size,
                                                                           item_size))
                               {
                                   data = asset;
                                   changed = true;
                                   ImGui::CloseCurrentPopup();
                               }
                           });

        ImGui::EndPopup();
    }

    return changed;
}

} // namespace

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
    auto& am = ctx.get<asset_manager>();
    auto& ths = ctx.get<thumbnail_system>();

    bool changed = pick_asset(ths, am, data, "Texture");

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
    auto& ths = ctx.get<thumbnail_system>();
    bool changed = pick_asset(ths, am, data, "Material");

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
        asset_writer::save_to_file(data.name(), data);
    }
    ImGui::Separator();
    {
        rttr::variant vari = &data.get();
        changed |= inspect_var(ctx, vari);
    }
    ImGui::Separator();
    if(ImGui::Button("SAVE CHANGES##bottom", ImVec2(-1, 0)))
    {
        asset_writer::save_to_file(data.name(), data);
    }
    return changed;
}

bool inspector_asset_handle_mesh::inspect_as_property(rtti::context& ctx, asset_handle<mesh>& data)
{
    auto& am = ctx.get<asset_manager>();
    auto& ths = ctx.get<thumbnail_system>();

    bool changed = pick_asset(ths, am, data, "Mesh");

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_mesh::inspect(rtti::context& ctx,
                                          rttr::variant& var,
                                          const var_info& info,
                                          const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<mesh>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    auto& am = ctx.get<asset_manager>();
    bool changed = false;

    if(ImGui::BeginTabBar("asset_handle_mesh",
                          ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll))
    {
        if(ImGui::BeginTabItem("Info"))
        {
            if(data)
            {
                const auto& mesh = data.get();
                mesh::info info;
                info.vertices = mesh.get_vertex_count();
                info.primitives = mesh.get_face_count();
                info.subsets = static_cast<std::uint32_t>(mesh.get_subset_count());
                rttr::variant vari = info;
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

bool inspector_asset_handle_animation::inspect_as_property(rtti::context& ctx, asset_handle<animation>& data)
{
    auto& am = ctx.get<asset_manager>();
    auto& ths = ctx.get<thumbnail_system>();

    bool changed = pick_asset(ths, am, data, "Animation Clip");

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_animation::inspect(rtti::context& ctx,
                                               rttr::variant& var,
                                               const var_info& info,
                                               const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<animation>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    auto& am = ctx.get<asset_manager>();
    bool changed = false;

    if(ImGui::BeginTabBar("asset_handle_animation",
                          ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll))
    {
        if(ImGui::BeginTabItem("Info"))
        {
            if(data)
            {
                //                rttr::variant vari = &data.get();
                //                changed |= inspect_var(ctx, vari);
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

} // namespace ace
