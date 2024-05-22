#include "inspector_assets.h"
#include "imgui/imgui.h"
#include "inspectors.h"

#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/assets/impl/asset_writer.h>
#include <engine/audio/audio_clip.h>
#include <engine/physics/physics_material.h>

#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>

#include <editor/editing/editing_manager.h>
#include <editor/editing/thumbnail_manager.h>

#include <filesystem/filesystem.h>
#include <filesystem/watcher.h>
#include <graphics/texture.h>
#include <imgui/imgui_internal.h>
#include <logging/logging.h>

namespace ace
{
namespace
{
auto resolve_path(const std::string& key) -> fs::path
{
    return fs::absolute(fs::resolve_protocol(key).string());
}

template<typename T>
auto reimport(const asset_handle<T>& asset)
{
    fs::watcher::touch(resolve_path(asset.id()), false);
}

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
                auto entry_future = am.template find_asset<T>(key);
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
            em.focus_path(fs::resolve_protocol(fs::path(data.id()).parent_path()));
        }

        ImGui::SameLine();
    }

    std::string item = data ? data.name() : fmt::format("None ({})", type);
    ImGui::AlignTextToFramePadding();

    auto popup_name = fmt::format("Pick {}", type);
    if(ImGui::Selectable(item.c_str(), false, ImGuiSelectableFlags_DontClosePopups))
    {
        filter.Clear();
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size * 0.4f);
        ImGui::OpenPopup(popup_name.c_str());
    }

    bool changed = false;

    bool open = true;
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

        ImGui::BeginChild("##items", {-1.0f, -1.0f});
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

        ImGui::EndChild();

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
        const auto tex = data.get_ptr();
        if(tex)
        {
            static auto t = tex->native_handle().idx;
            static int mip = 0;

            if(t != tex->native_handle().idx)
            {
                t = tex->native_handle().idx;
                mip = 0;
            }

            auto sz = ImGui::GetSize(data, size);
            ImGui::ImageWithAspect(ImGui::ToId(tex, mip), sz, size);
            ImGui::SliderInt("Mip", &mip, 0, tex->info.numMips - 1);
            return;
        }
    }

    ImGui::Dummy(size);
    ImGui::RenderFrameBorder(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
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
                const auto tex = data.get_ptr();
                if(tex)
                {
                    changed |= ::ace::inspect(ctx, tex->info);
                }
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Import"))
        {
            ImGui::TextUnformatted("Import options");

            if(ImGui::Button("Reimport"))
            {
                reimport(data);
            }
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

    bool changed = false;

    if(ImGui::Button("SAVE CHANGES##top", ImVec2(-1, 0)))
    {
        asset_writer::save_to_file(data.id(), data);
    }
    ImGui::Separator();
    {
        auto var = data.get_ptr();
        if(var)
        {
            changed |= ::ace::inspect(ctx, *var);
        }

        if(changed)
        {
            auto& tm = ctx.get<thumbnail_manager>();
            tm.regenerate_thumbnail(data.uid());
        }
    }
    ImGui::Separator();
    if(ImGui::Button("SAVE CHANGES##bottom", ImVec2(-1, 0)) || changed)
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
                changed |= ::ace::inspect(ctx, info);
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Import"))
        {
            ImGui::TextUnformatted("Import options");

            if(ImGui::Button("Reimport"))
            {
                reimport(data);
            }
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

            if(ImGui::Button("Reimport"))
            {
                reimport(data);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    return changed;
}

bool inspector_asset_handle_prefab::inspect_as_property(rtti::context& ctx, asset_handle<prefab>& data)
{
    auto& am = ctx.get<asset_manager>();
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Prefab");

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_prefab::inspect(rtti::context& ctx,
                                            rttr::variant& var,
                                            const var_info& info,
                                            const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<prefab>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    auto& am = ctx.get<asset_manager>();
    bool changed = false;

    if(ImGui::BeginTabBar("asset_handle_prefab",
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

            if(ImGui::Button("Reimport"))
            {
                reimport(data);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    return changed;
}

bool inspector_asset_handle_scene_prefab::inspect_as_property(rtti::context& ctx, asset_handle<scene_prefab>& data)
{
    auto& am = ctx.get<asset_manager>();
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Scene");

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_scene_prefab::inspect(rtti::context& ctx,
                                                  rttr::variant& var,
                                                  const var_info& info,
                                                  const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<scene_prefab>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    auto& am = ctx.get<asset_manager>();
    bool changed = false;

    if(ImGui::BeginTabBar("asset_handle_scene_prefab",
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

            if(ImGui::Button("Reimport"))
            {
                reimport(data);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    return changed;
}

bool inspector_asset_handle_physics_material::inspect_as_property(rtti::context& ctx,
                                                                  asset_handle<physics_material>& data)
{
    auto& am = ctx.get<asset_manager>();
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Physics Material");

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_physics_material::inspect(rtti::context& ctx,
                                                      rttr::variant& var,
                                                      const var_info& info,
                                                      const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<physics_material>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    bool changed = false;

    if(ImGui::Button("SAVE CHANGES##top", ImVec2(-1, 0)))
    {
        asset_writer::save_to_file(data.id(), data);
    }
    ImGui::Separator();
    {
        auto var = data.get_ptr();
        if(var)
        {
            changed |= ::ace::inspect(ctx, *var);
        }
    }
    ImGui::Separator();
    if(ImGui::Button("SAVE CHANGES##bottom", ImVec2(-1, 0)) || changed)
    {
        asset_writer::save_to_file(data.id(), data);
    }
    return changed;
}

bool inspector_asset_handle_audio_clip::inspect_as_property(rtti::context& ctx, asset_handle<audio_clip>& data)
{
    auto& am = ctx.get<asset_manager>();
    auto& tm = ctx.get<thumbnail_manager>();
    auto& em = ctx.get<editing_manager>();

    bool changed = pick_asset(filter, em, tm, am, data, "Audio Clip");

    if(process_drag_drop_target(am, data))
    {
        changed = true;
    }

    return changed;
}

bool inspector_asset_handle_audio_clip::inspect(rtti::context& ctx,
                                                rttr::variant& var,
                                                const var_info& info,
                                                const meta_getter& get_metadata)
{
    auto& data = var.get_value<asset_handle<audio_clip>>();

    if(info.is_property)
    {
        return inspect_as_property(ctx, data);
    }

    auto& am = ctx.get<asset_manager>();
    bool changed = false;

    {
        auto var = data.get_ptr();
        if(var)
        {
            const auto& info = var->get_info();
            changed |= ::ace::inspect(ctx, info);
        }
    }

    return changed;
}

} // namespace ace
