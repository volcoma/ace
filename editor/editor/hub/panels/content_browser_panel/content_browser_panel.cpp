#include "content_browser_panel.h"
#include "../panels_defs.h"

#include <editor/editing/editing_manager.h>
#include <editor/editing/thumbnail_manager.h>
#include <editor/imgui/integration/fonts/icons/icons_material_design_icons.h>

#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_extensions.h>
#include <engine/assets/impl/asset_writer.h>
#include <engine/ecs/components/id_component.h>
#include <engine/meta/ecs/entity.hpp>
#include <engine/physics/physics_material.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/renderer.h>

#include <engine/audio/audio_clip.h>

#include <filedialog/filedialog.h>
#include <filesystem/watcher.h>
#include <hpp/utility.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <logging/logging.h>

namespace ace
{
using namespace std::literals;
namespace
{

ImGuiKey rename_key = ImGuiKey_F2;
ImGuiKey delete_key = ImGuiKey_Delete;
ImGuiKeyCombination duplicate_combination{ImGuiKey_LeftCtrl, ImGuiKey_D};

auto get_new_file(const fs::path& path, const std::string& name, const std::string& ext = "") -> fs::path
{
    int i = 0;
    fs::error_code err;
    while(fs::exists(path / (fmt::format("{} ({})", name.c_str(), i) + ext), err))
    {
        ++i;
    }

    return path / (fmt::format("{} ({})", name.c_str(), i) + ext);
}

auto process_drag_drop_source(const gfx::texture::ptr& preview, const fs::path& absolute_path) -> bool
{
    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        const auto filename = absolute_path.filename();
        const std::string extension = filename.has_extension() ? filename.extension().string() : "folder";
        const std::string id = absolute_path.string();
        const std::string strfilename = filename.string();
        ImVec2 item_size = {64, 64};
        ImVec2 texture_size = ImGui::GetSize(preview);
        texture_size = ImMax(texture_size, item_size);

        ImGui::ImageButtonWithAspectAndTextBelow(ImGui::ToId(preview), strfilename, texture_size, item_size);

        ImGui::SetDragDropPayload(extension.c_str(), id.data(), id.size());
        ImGui::EndDragDropSource();
        return true;
    }

    return false;
}

void process_drag_drop_target(const fs::path& absolute_path)
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

        fs::error_code err;
        if(fs::is_directory(absolute_path, err))
        {
            static const auto types = ex::get_all_formats();

            const auto process_drop = [&absolute_path](const std::string& type)
            {
                auto payload = ImGui::AcceptDragDropPayload(type.c_str());
                if(payload != nullptr)
                {
                    std::string data(reinterpret_cast<const char*>(payload->Data), std::size_t(payload->DataSize));
                    fs::path new_name = absolute_path / fs::path(data).filename();
                    if(data != new_name)
                    {
                        fs::error_code err;

                        if(!fs::exists(new_name, err))
                        {
                            fs::rename(data, new_name, err);
                        }
                    }
                }
                return payload;
            };

            for(const auto& asset_set : types)
            {
                for(const auto& type : asset_set)
                {
                    if(process_drop(type) != nullptr)
                    {
                        break;
                    }
                }
            }
            {
                process_drop("folder");
            }
            {
                {
                    auto payload = ImGui::AcceptDragDropPayload("entity");
                    if(payload != nullptr)
                    {
                        entt::handle dropped{};
                        std::memcpy(&dropped, payload->Data, size_t(payload->DataSize));
                        if(dropped)
                        {
                            auto& tag = dropped.get<tag_component>();

                            auto prefab_path = absolute_path / fs::path(tag.tag + ".pfb").make_preferred();
                            save_to_file(prefab_path.string(), dropped);
                        }
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

auto draw_item(const content_browser_item& item)
{
    const auto& absolute_path = item.entry.entry.path();
    const auto& name = item.entry.stem;
    const auto& filename = item.entry.filename;
    const auto& file_ext = item.entry.extension;

    enum class entry_action
    {
        none,
        clicked,
        double_clicked,
        renamed,
        deleted,
        duplicate,
    };

    auto duplicate_entry = [&]()
    {
        fs::error_code err;
        const auto available = get_new_file(absolute_path.parent_path(), name, file_ext);
        fs::copy(absolute_path, available, fs::copy_options::overwrite_existing, err);
    };

    bool is_popup_opened = false;
    entry_action action = entry_action::none;

    bool open_rename_menu = false;

    ImGui::PushID(name.c_str());
    if(item.is_selected && !ImGui::IsAnyItemActive() && ImGui::IsWindowFocused())
    {
        if(ImGui::IsKeyPressed(rename_key))
        {
            open_rename_menu = true;
        }

        if(ImGui::IsKeyPressed(delete_key))
        {
            action = entry_action::deleted;
        }

        if(ImGui::IsItemCombinationKeyPressed(duplicate_combination))
        {
            action = entry_action::duplicate;
        }
    }

    ImVec2 item_size = {item.size, item.size};
    ImVec2 texture_size = ImGui::GetSize(item.icon, item_size);

    auto col = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x, col.y, col.z, 0.44f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x, col.y, col.z, 0.86f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(col.x, col.y, col.z, 1.0f));

    auto pos = ImGui::GetCursorScreenPos();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    if(ImGui::ImageButtonWithAspectAndTextBelow(ImGui::ToId(item.icon), name, texture_size, item_size))
    {
        action = entry_action::clicked;
    }
    ImGui::DrawItemActivityOutline(ImGui::OutlineFlags_All);
    pos.y += ImGui::GetItemRectSize().y;

    ImGui::PopStyleVar();

    ImGui::PopStyleColor(3);

    if(ImGui::IsItemHovered())
    {
        if(item.on_double_click)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        if(ImGui::IsMouseDoubleClicked(0))
        {
            action = entry_action::double_clicked;
        }
    }

    ImGui::ItemTooltip(filename.c_str());

    if(!file_ext.empty())
    {
        ImGui::PushFont(ImGui::GetFont(ImGui::Font::Black));
        ImGui::ItemTooltip(ex::get_type(file_ext).c_str());
        ImGui::PopFont();
    }

    auto input_buff = ImGui::CreateInputTextBuffer(name);

    if(ImGui::BeginPopupContextItem("ENTRY_CONTEXT_MENU"))
    {
        is_popup_opened = true;

        if(ImGui::MenuItem("Rename", ImGui::GetKeyName(rename_key)))
        {
            open_rename_menu = true;
            ImGui::CloseCurrentPopup();
        }

        if(ImGui::MenuItem("Duplicate", ImGui::GetKeyCombinationName(duplicate_combination).c_str()))
        {
            action = entry_action::duplicate;
            ImGui::CloseCurrentPopup();
        }

        if(ImGui::MenuItem("Delete", ImGui::GetKeyName(delete_key)))
        {
            action = entry_action::deleted;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    const float rename_field_width = 150.0f;
    if(open_rename_menu)
    {
        ImGui::OpenPopup("ENTRY_RENAME_MENU");

        const auto& style = ImGui::GetStyle();
        float renameFieldWithPadding = rename_field_width + style.WindowPadding.x * 2.0f;
        if(item.size < renameFieldWithPadding)
        {
            auto diff = renameFieldWithPadding - item.size;
            pos.x -= diff * 0.5f;
        }

        ImGui::SetNextWindowPos(pos);
    }

    if(ImGui::BeginPopup("ENTRY_RENAME_MENU"))
    {
        is_popup_opened = true;
        if(open_rename_menu)
        {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::PushItemWidth(rename_field_width);

        if(ImGui::InputTextWidget("##NAME",
                                  input_buff,
                                  false,
                                  ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            action = entry_action::renamed;
            ImGui::CloseCurrentPopup();
        }

        if(open_rename_menu)
        {
            ImGui::ActivateItemByID(ImGui::GetItemID());
        }
        ImGui::PopItemWidth();
        ImGui::EndPopup();
    }
    if(item.is_selected)
    {
        ImGui::SetItemFocusFrame();
    }

    if(item.is_focused)
    {
        ImGui::SetItemFocusFrame(ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 0.0f, 1.0f)));
    }

    if(item.is_loading)
    {
        action = entry_action::none;
    }

    switch(action)
    {
        case entry_action::clicked:
        {
            if(item.on_click)
            {
                item.on_click();
            }
        }
        break;
        case entry_action::double_clicked:
        {
            if(item.on_double_click)
            {
                item.on_double_click();
            }
        }
        break;
        case entry_action::renamed:
        {
            const std::string new_name = std::string(input_buff.data());
            if(new_name != name && !new_name.empty())
            {
                if(item.on_rename)
                {
                    item.on_rename(new_name);
                }
            }
        }
        break;
        case entry_action::deleted:
        {
            if(item.on_delete)
            {
                item.on_delete();
            }
        }
        break;

        case entry_action::duplicate:
        {
            duplicate_entry();
        }
        break;

        default:
            break;
    }

    if(!process_drag_drop_source(item.icon, absolute_path))
    {
        process_drag_drop_target(absolute_path);
    }

    ImGui::PopID();
    return is_popup_opened;
}

} // namespace

void content_browser_panel::init(rtti::context& ctx)
{
}

void content_browser_panel::deinit(rtti::context& ctx)
{
    filter_ = {};
}

void content_browser_panel::on_frame_ui_render(rtti::context& ctx, const char* name)
{
    if(ImGui::Begin(name, nullptr))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));

        draw(ctx);
    }
    ImGui::End();
}

void content_browser_panel::draw(rtti::context& ctx)
{
    auto& em = ctx.get<editing_manager>();

    const auto root_path = fs::resolve_protocol("app:/data");

    fs::error_code err;
    if(root_ != root_path || !fs::exists(cache_.get_path(), err))
    {
        root_ = root_path;
        set_cache_path(root_);
    }

    if(!em.focused_data.focus_path.empty())
    {
        set_cache_path(em.focused_data.focus_path);
        em.focused_data.focus_path.clear();
    }

    auto avail = ImGui::GetContentRegionAvail();
    if(avail.x < 1.0f || avail.y < 1.0f)
    {
        return;
    }

    if(ImGui::BeginChild("DETAILS_AREA", avail * ImVec2(0.15f, 1.0f), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));

        if(fs::is_directory(root_path, err))
        {
            draw_details(ctx, root_path);
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if(ImGui::BeginChild("EXPLORER"))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));
        draw_as_explorer(ctx, root_path);
    }
    ImGui::EndChild();

    const auto& current_path = cache_.get_path();
    process_drag_drop_target(current_path);

    if(refresh_ > 0)
    {
        refresh_--;
    }
}

void content_browser_panel::draw_details(rtti::context& ctx, const fs::path& path)
{
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

        const auto& selected_path = cache_.get_path();
        if(selected_path == path)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if(refresh_ > 0 && (path == selected_path || fs::is_any_parent_path(path, selected_path)))
        {
            ImGui::SetNextItemOpen(true);
        }

        auto stem = path.stem();
        bool open = ImGui::TreeNodeEx(fmt::format("{} {}", ICON_MDI_FOLDER, stem.generic_string()).c_str(), flags);
        process_drag_drop_target(path);

        const bool clicked = !ImGui::IsItemToggledOpen() && ImGui::IsItemClicked(ImGuiMouseButton_Left);

        if(open)
        {
            const fs::directory_iterator it(path);
            for(const auto& p : it)
            {
                if(fs::is_directory(p.status()))
                {
                    const auto& path = p.path();
                    draw_details(ctx, path);
                }
            }

            ImGui::TreePop();
        }

        if(clicked)
        {
            set_cache_path(path);
        }
    }
}

void content_browser_panel::draw_as_explorer(rtti::context& ctx, const fs::path& root_path)
{
    auto& am = ctx.get<asset_manager>();
    auto& em = ctx.get<editing_manager>();
    auto& tm = ctx.get<thumbnail_manager>();

    const float size = ImGui::GetFrameHeight() * 6.0f * scale_;
    const auto hierarchy = fs::split_until(cache_.get_path(), root_path);

    ImGui::DrawFilterWithHint(filter_, ICON_MDI_FILE_SEARCH " Search...", 200.0f);
    ImGui::DrawItemActivityOutline();
    ImGui::SameLine();

    int id = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));

    for(const auto& dir : hierarchy)
    {
        const bool is_first = &dir == &hierarchy.front();
        const bool is_last = &dir == &hierarchy.back();
        ImGui::PushID(id++);

        if(!is_first)
        {
            ImGui::SameLine(0.0f, 0.0f);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("/");
            ImGui::SameLine(0.0f, 0.0f);
        }

        if(is_last)
        {
            ImGui::PushFont(ImGui::Font::Bold);
        }

        const bool clicked = ImGui::Button(dir.filename().string().c_str());

        if(is_last)
        {
            ImGui::PopFont();
        }
        ImGui::PopID();

        if(clicked)
        {
            set_cache_path(dir);
            break;
        }
        process_drag_drop_target(dir);
    }
    ImGui::PopStyleVar(2);

    ImGui::SameLine(0.0f, 0.0f);
    ImGui::AlignedItem(1.0f,
                       ImGui::GetContentRegionAvail().x,
                       80.0f,
                       [&]()
                       {
                           ImGui::PushItemWidth(80.0f);
                           ImGui::SliderFloat("##scale", &scale_, 0.5f, 1.0f);
                           ImGui::SetItemTooltip("%s", "Icons scale");
                           ImGui::PopItemWidth();
                       });

    ImGui::Separator();

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoSavedSettings;

    fs::path current_path = cache_.get_path();

    if(ImGui::BeginChild("assets_content", ImGui::GetContentRegionAvail(), false, flags))
    {
        ImGui::PushWindowFontSize(16);

        bool is_popup_opened = false;

        auto process_cache_entry = [&](const auto& cache_entry)
        {
            const auto& absolute_path = cache_entry.entry.path();
            const auto& name = cache_entry.stem;
            const auto& filename = cache_entry.filename;
            const auto& relative = cache_entry.protocol_path;
            const auto& file_ext = cache_entry.extension;

            const auto on_rename = [&](const std::string& new_name)
            {
                fs::path new_absolute_path = absolute_path;
                new_absolute_path.remove_filename();
                new_absolute_path /= new_name + file_ext;
                fs::error_code err;
                fs::rename(absolute_path, new_absolute_path, err);
            };

            const auto on_delete = [&]()
            {
                fs::error_code err;
                fs::remove_all(absolute_path, err);

                em.unselect();
            };

            content_browser_item item(cache_entry);
            item.on_rename = on_rename;
            item.on_delete = on_delete;
            item.size = size;

            bool known = false;
            hpp::for_each_type<gfx::texture,
                               gfx::shader,
                               scene_prefab,
                               material,
                               physics_material,
                               audio_clip,
                               mesh,
                               prefab,
                               animation>(
                [&](auto tag)
                {
                    if(known)
                    {
                        return;
                    }

                    using asset_t = typename std::decay_t<decltype(tag)>::type;

                    if(ex::is_format<asset_t>(file_ext))
                    {
                        known = true;
                        using entry_t = asset_handle<asset_t>;
                        const auto& entry = am.find_asset<asset_t>(relative);

                        item.icon = tm.get_thumbnail(entry);
                        item.is_selected = em.is_selected(entry);
                        item.is_focused = em.is_focused(entry);
                        item.is_loading = !entry.is_ready();
                        item.on_click = [&]()
                        {
                            em.select(entry);
                        };

                        if constexpr(std::is_same_v<asset_t, scene_prefab>)
                        {
                            item.on_double_click = [&]()
                            {
                                auto& ec = ctx.get<ecs>();
                                auto& scene = ec.get_scene();
                                scene.load_from(entry);
                            };
                        }

                        is_popup_opened |= draw_item(item);
                    }
                });

            if(!known)
            {
                fs::error_code ec;
                using entry_t = fs::path;
                const entry_t& entry = absolute_path;
                item.icon = tm.get_thumbnail(entry);
                item.is_selected = em.is_selected(entry);
                item.is_focused = em.is_focused(entry);

                item.on_click = [&]()
                {
                    em.select(entry);
                };

                if(fs::is_directory(cache_entry.entry.status()))
                {
                    item.on_double_click = [&]()
                    {
                        current_path = entry;
                        em.try_unselect<entry_t>();
                    };
                }

                is_popup_opened |= draw_item(item);
            }
        };

        auto cache_size = cache_.size();

        if(!filter_.IsActive())
        {
            ImGui::ItemBrowser(size,
                               cache_size,
                               [&](int index)
                               {
                                   auto& cache_entry = cache_[index];
                                   process_cache_entry(cache_entry);
                               });
        }
        else
        {
            std::vector<fs::directory_cache::cache_entry> filtered_entries;
            for(size_t index = 0; index < cache_size; ++index)
            {
                const auto& cache_entry = cache_[index];

                const auto& name = cache_entry.stem;
                const auto& filename = cache_entry.filename;

                if(filter_.PassFilter(name.c_str()))
                {
                    filtered_entries.emplace_back(cache_entry);
                }
            }

            ImGui::ItemBrowser(size,
                               filtered_entries.size(),
                               [&](int index)
                               {
                                   auto& cache_entry = filtered_entries[index];
                                   process_cache_entry(cache_entry);
                               });
        }

        if(!is_popup_opened)
        {
            context_menu(ctx);
        }
        set_cache_path(current_path);

        ImGui::PopWindowFontSize();
    }
    ImGui::EndChild();
}

void content_browser_panel::context_menu(rtti::context& ctx)
{
    if(ImGui::BeginPopupContextWindow())
    {
        context_create_menu(ctx);

        ImGui::Separator();

        if(ImGui::Selectable("Open in Explorer"))
        {
            fs::show_in_graphical_env(cache_.get_path());
        }

        ImGui::Separator();

        if(ImGui::Selectable("Import..."))
        {
            import(ctx);
        }
        ImGui::ItemTooltip("If import asset consists of multiple files,\n"
                           "just copy paste all the files the data folder.\n"
                           "Preferably in a new folder. The importer will\n"
                           "automatically pick them up as dependencies.");

        ImGui::EndPopup();
    }
}

void content_browser_panel::context_create_menu(rtti::context& ctx)
{
    if(ImGui::BeginMenu("Create"))
    {
        if(ImGui::MenuItem("Folder"))
        {
            const auto available = get_new_file(cache_.get_path(), "New Folder");
            fs::error_code err;
            fs::create_directory(available, err);
        }

        ImGui::Separator();

        if(ImGui::MenuItem("Material"))
        {
            auto& am = ctx.get<asset_manager>();

            const auto available = get_new_file(cache_.get_path(), "New Material", ex::get_format<material>());
            const auto key = fs::convert_to_protocol(available).generic_string();

            auto new_mat_future = am.get_asset_from_instance<material>(key, std::make_shared<pbr_material>());
            asset_writer::save_to_file(new_mat_future.id(), new_mat_future);
        }

        if(ImGui::MenuItem("Physics Material"))
        {
            auto& am = ctx.get<asset_manager>();

            const auto available =
                get_new_file(cache_.get_path(), "New Physics Material", ex::get_format<physics_material>());
            const auto key = fs::convert_to_protocol(available).generic_string();

            auto new_mat_future =
                am.get_asset_from_instance<physics_material>(key, std::make_shared<physics_material>());
            asset_writer::save_to_file(new_mat_future.id(), new_mat_future);
        }

        ImGui::EndMenu();
    }
}

void content_browser_panel::set_cache_path(const fs::path& path)
{
    if(cache_.get_path() == path)
    {
        return;
    }
    cache_.set_path(path);
    refresh_ = 3;
}

void content_browser_panel::import(rtti::context& ctx)
{
    std::vector<std::string> paths;
    if(native::open_files_dialog(paths, {}))
    {
        on_import(ctx, paths);
    }
}

void content_browser_panel::on_import(rtti::context& ctx, const std::vector<std::string>& paths)
{
    auto& ts = ctx.get<threader>();

    for(auto& path : paths)
    {
        fs::path p = fs::path(path).make_preferred();
        fs::path filename = p.filename();

        auto task = ts.pool->schedule(
            [opened = cache_.get_path()](const fs::path& path, const fs::path& filename)
            {
                fs::error_code err;
                fs::path dir = opened / filename;
                fs::copy_file(path, dir, fs::copy_options::overwrite_existing, err);
            },
            p,
            filename);
    }
}
} // namespace ace
