#include "inspector_assets.h"
#include "imgui/imgui.h"
#include "inspectors.h"

#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_writer.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>

#include <editor/editing/thumbnail_manager.h>
#include <editor/editing/editing_manager.h>

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
bool pick_asset(ImGuiTextFilter& filter,
                editing_manager& em,
                thumbnail_manager& tm,
                asset_manager& am,
                asset_handle<T>& data,
                const std::string& type)
{
    if(data)
    {
        const auto& thumbnail = tm.get_thumbnail(data);

        auto fh = ImGui::GetFrameHeight();
        ImVec2 item_size = ImVec2(fh, fh);
        ImVec2 texture_size = ImGui::GetSize(thumbnail, item_size);

        if(ImGui::ImageButtonWithAspectAndTextBelow(ImGui::ToId(thumbnail), {}, texture_size, item_size))
        {
            em.focus(data);
        }

        ImGui::SameLine();
    }

    std::string item = data ? data.name() : fmt::format("None ({})", type);
    ImGui::AlignTextToFramePadding();

    auto popup_name = fmt::format("Pick {}", type);
    if(ImGui::Selectable(item.c_str()))
    {
        ImGui::OpenPopup(popup_name.c_str());
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.4f);
        filter.Clear();
    }

    bool changed = false;

    bool open = ImGui::IsPopupOpen(popup_name.c_str());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
    if(ImGui::BeginPopupModal(popup_name.c_str(), &open))
    {
        if(!open)
        {
            ImGui::CloseCurrentPopup();
        }

        if(ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere();
        }

        filter.Draw("##Filter", ImGui::GetContentRegionAvail().x);

        auto assets = am.get_assets<T>(
            [&](const auto& asset)
            {
                return filter.PassFilter(asset.name().c_str());
            });

        const float size = 100.0f;
        ImGui::ItemBrowser(size,
                           assets.size(),
                           [&](int index)
                           {
                               auto& asset = assets[index];
                               const auto& thumbnail = tm.get_thumbnail(asset);

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

                               ImGui::ItemTooltip(asset.name().c_str());
                           });

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    return changed;
}

} // namespace

void inspector_asset_handle_texture::draw_image(const asset_handle<gfx::texture>& data, ImVec2 size) const
{
    if(data.is_ready())
    {
        const auto& tex = data.get();
        static auto t = tex.native_handle().idx;
        static int mip = 0;

        if(t != tex.native_handle().idx)
        {
            t = tex.native_handle().idx;
            mip = 0;
        }

        auto sz = ImGui::GetSize(data, size);
        ImGui::ImageWithAspect(ImGui::ToId(data, mip), sz, size);
        ImGui::SliderInt("Mip", &mip, 0, tex.info.numMips - 1);
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
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Texture");

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
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Material");

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

bool inspector_asset_handle_mesh::inspect_as_property(rtti::context& ctx, asset_handle<mesh>& data)
{
    auto& am = ctx.get<asset_manager>();
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Mesh");

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
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Animation Clip");

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