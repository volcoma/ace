#include "content_browser.h"

#include <editor/assets/asset_extensions.h>
#include <editor/editing/editing_manager.h>
#include <editor/editing/thumbnail_manager.h>

#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/assets/impl/asset_writer.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/renderer.h>

#include <filedialog/filedialog.h>
#include <hpp/utility.hpp>
#include <imgui/imgui_internal.h>
#include <logging/logging.h>

#include <chrono>

namespace ace
{
using namespace std::literals;
namespace
{
auto process_drag_drop_source(const asset_handle<gfx::texture>& preview, const fs::path& absolute_path) -> bool
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
            //            {
            //                auto payload = ImGui::AcceptDragDropPayload("entity");
            //                if(payload != nullptr)
            //                {
            //                    std::uint32_t entity_index = 0;
            //                    std::memcpy(&entity_index, payload->Data, std::size_t(payload->DataSize));
            //                    auto& ecs = core::get_subsystem<runtime::entity_component_system>();
            //                    if(ecs.valid_index(entity_index))
            //                    {
            //                        auto eid = ecs.create_id(entity_index);
            //                        auto dropped_entity = ecs.get(eid);
            //                        if(dropped_entity)
            //                        {
            //                            auto prefab_path =
            //                                absolute_path / fs::path(dropped_entity.to_string() +
            //                                ".pfb").make_preferred();
            //                            ecs::utils::save_entity_to_file(prefab_path, dropped_entity);
            //                        }
            //                    }
            //                }
            //            }
        }
        ImGui::EndDragDropTarget();
    }
}

auto draw_entry(const asset_handle<gfx::texture>& icon,
                bool is_loading,
                const std::string& name,
                const fs::path& absolute_path,
                bool is_selected,
                const float size,
                const std::function<void()>& on_click,
                const std::function<void()>& on_double_click,
                const std::function<void(const std::string&)>& on_rename,
                const std::function<void()>& on_delete) -> bool
{
    enum class entry_action
    {
        none,
        clicked,
        double_clicked,
        renamed,
        deleted,
    };

    bool is_popup_opened = false;
    entry_action action = entry_action::none;

    bool open_rename_menu = false;

    ImGui::PushID(name.c_str());
    if(is_selected && !ImGui::IsAnyItemActive() && ImGui::IsWindowFocused())
    {
        if(ImGui::IsKeyPressed(ImGuiKey_F2))
        {
            open_rename_menu = true;
        }

        if(ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            action = entry_action::deleted;
        }
    }

    ImVec2 item_size = {size, size};
    ImVec2 texture_size = ImGui::GetSize(icon, item_size);

    auto col = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(col.x, col.y, col.z, 0.44f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(col.x, col.y, col.z, 0.86f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(col.x, col.y, col.z, 1.0f));

    auto pos = ImGui::GetCursorScreenPos();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    if(ImGui::ImageButtonWithAspectAndTextBelow(ImGui::ToId(icon), name, texture_size, item_size))
    {
        action = entry_action::clicked;
    }
    pos.y += ImGui::GetItemRectSize().y;

    ImGui::PopStyleVar();

    ImGui::PopStyleColor(3);

    if(ImGui::IsItemHovered())
    {
        if(on_double_click)
        {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
        if(ImGui::IsMouseDoubleClicked(0))
        {
            action = entry_action::double_clicked;
        }

    }

    ImGui::ItemTooltip(name.c_str());


    //    if(is_selected && ImGui::GetNavInputAmount(ImGuiNavInput_Input, ImGuiInputReadMode_Pressed) > 0.0f)
    //    {
    //        action = entry_action::double_clicked;
    //    }

    auto input_buff = ImGui::CreateInputTextBuffer(name);

    if(ImGui::BeginPopupContextItem("ENTRY_CONTEXT_MENU"))
    {
        is_popup_opened = true;

        if(ImGui::MenuItem("RENAME", "F2"))
        {
            open_rename_menu = true;
            ImGui::CloseCurrentPopup();
        }

        if(ImGui::MenuItem("DELETE", "DEL"))
        {
            action = entry_action::deleted;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if(open_rename_menu)
    {
        ImGui::OpenPopup("ENTRY_RENAME_MENU");
        ImGui::SetNextWindowPos(pos);
    }

    if(ImGui::BeginPopup("ENTRY_RENAME_MENU"))
    {
        is_popup_opened = true;
        if(open_rename_menu)
        {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::PushItemWidth(150.0f);
        if(ImGui::InputText("##NAME",
                            input_buff.data(),
                            input_buff.size(),
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
    if(is_selected)
    {
        ImGui::RenderFocusFrame(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    }

    if(is_loading)
    {
        action = entry_action::none;
    }

    switch(action)
    {
        case entry_action::clicked:
        {
            if(on_click)
            {
                on_click();
            }
        }
        break;
        case entry_action::double_clicked:
        {
            if(on_double_click)
            {
                on_double_click();
            }
        }
        break;
        case entry_action::renamed:
        {
            std::string new_name = std::string(input_buff.data());
            if(new_name != name && !new_name.empty())
            {
                if(on_rename)
                {
                    on_rename(new_name);
                }
            }
        }
        break;
        case entry_action::deleted:
        {
            if(on_delete)
            {
                on_delete();
            }
        }
        break;

        default:
            break;
    }

    if(!process_drag_drop_source(icon, absolute_path))
    {
        process_drag_drop_target(absolute_path);
    }

    ImGui::PopID();
    return is_popup_opened;
}

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

} // namespace

void content_browser::init(rtti::context& ctx)
{
}

void content_browser::draw(rtti::context& ctx)
{

    const auto root_path = fs::resolve_protocol("app:/data");

    fs::error_code err;
    if(root_ != root_path || !fs::exists(cache_.get_path(), err))
    {
        root_ = root_path;
        cache_.set_path(root_);
    }

    auto avail = ImGui::GetContentRegionAvail();
    if(avail.x < 1.0f || avail.y < 1.0f)
    {
        return;
    }

    if(ImGui::BeginChild("DETAILS_AREA", avail * ImVec2(0.1f, 1.0f), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX))
    {
        draw_details(ctx, root_path);
    }
    ImGui::EndChild();

    ImGui::SameLine();

    if(ImGui::BeginChild("EXPLORER"))
    {
        draw_as_explorer(ctx, root_path);
    }
    ImGui::EndChild();

    const auto& current_path = cache_.get_path();
    process_drag_drop_target(current_path);
}

void content_browser::draw_details(rtti::context& ctx, const fs::path& root_path)
{
}

void content_browser::draw_as_explorer(rtti::context& ctx, const fs::path& root_path)
{
    auto& am = ctx.get<asset_manager>();
    auto& em = ctx.get<editing_manager>();
    auto& tm = ctx.get<thumbnail_manager>();

    ImGui::PushItemWidth(80.0f);
    ImGui::SliderFloat("##scale", &scale_, 0.5f, 1.0f);
    const float size = ImGui::GetFrameHeight() * 6.0f * scale_;
    if(ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::TextUnformatted("SCALE ICONS");
        ImGui::EndTooltip();
    }
    ImGui::PopItemWidth();

    const auto hierarchy = fs::split_until(cache_.get_path(), root_path);

    int id = 0;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));

    for(const auto& dir : hierarchy)
    {
        bool is_first = &dir == &hierarchy.front();
        bool is_last = &dir == &hierarchy.back();
        ImGui::PushID(id++);

        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        if(!is_first)
        {
            ImGui::AlignTextToFramePadding();
            ImGui::Button(">");
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
        }

        if(is_last)
        {
            ImGui::PushFont(ImGui::Font::Bold);
        }

        bool clicked = ImGui::Button(dir.filename().string().c_str());

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
                fs::remove(absolute_path, err);

                em.unselect();
            };

            if(fs::is_directory(cache_entry.entry.status()))
            {
                fs::error_code ec;
                using entry_t = fs::path;
                const entry_t& entry = absolute_path;
                const auto& icon = tm.get_thumbnail(entry);
                bool selected = em.is_selected(entry);
                is_popup_opened |= draw_entry(
                    icon,
                    false,
                    name,
                    absolute_path,
                    selected,
                    size,
                    [&]() // on_click
                    {
                        em.select(entry);
                    },
                    [&]() // on_double_click
                    {
                        current_path = entry;
                        em.try_unselect<fs::path>();
                    },
                    [&](const std::string& new_name) // on_rename
                    {
                        fs::path new_absolute_path = absolute_path;
                        new_absolute_path.remove_filename();
                        new_absolute_path /= new_name;
                        fs::error_code err;
                        fs::rename(absolute_path, new_absolute_path, err);
                    },
                    [&]() // on_delete
                    {
                        fs::error_code err;
                        fs::remove_all(absolute_path, err);
                    });

                return;
            }


//            hpp::for_each_type<gfx::texture/*, gfx::shader*/>([&](const auto& t)
//            {
//                using asset_t = std::decay_t<decltype(t)>::type;

//                if(ex::is_format<asset_t>(file_ext))
//                {
//                    using entry_t = asset_handle<asset_t>;
//                    const auto& entry = am.find_asset_entry<asset_t>(relative);
//                    bool is_loading = !entry.is_ready();
//                    const auto& icon = tm.get_thumbnail(entry);
//                    bool selected = em.is_selected(entry);

//                    auto on_click = [&em, &entry]() mutable // on_click
//                    {
//                        em.select(entry);
//                    };
////                    auto on_click = nullptr;

//                    is_popup_opened |= draw_entry(
//                        icon,
//                        is_loading,
//                        name,
//                        absolute_path,
//                        selected,
//                        size,
//                        on_click,
//                        nullptr, // on_double_click
//                        on_rename,
//                        on_delete);
//                }
//            });

            if(ex::is_format<gfx::texture>(file_ext))
            {
                using asset_t = gfx::texture;
                using entry_t = asset_handle<asset_t>;
                const auto& entry = am.find_asset_entry<asset_t>(relative);
                bool is_loading = !entry.is_ready();
                const auto& icon = tm.get_thumbnail(entry);
                bool selected = em.is_selected(entry) || em.is_focused(entry);

                is_popup_opened |= draw_entry(
                    icon,
                    is_loading,
                    name,
                    absolute_path,
                    selected,
                    size,
                    [&]() // on_click
                    {
                        em.select(entry);
                    },
                    nullptr, // on_double_click
                    on_rename,
                    on_delete);
                return;
            }

            if(ex::is_format<gfx::shader>(file_ext))
            {
                using asset_t = gfx::shader;
                using entry_t = asset_handle<asset_t>;
                const auto& entry = am.find_asset_entry<asset_t>(relative);
                bool is_loading = !entry.is_ready();
                const auto& icon = tm.get_thumbnail(entry);
                bool selected = em.is_selected(entry);

                is_popup_opened |= draw_entry(
                    icon,
                    is_loading,
                    name,
                    absolute_path,
                    selected,
                    size,
                    [&]() // on_click
                    {
                        em.select(entry);
                    },
                    nullptr, // on_double_click
                    on_rename,
                    on_delete);
                return;
            }

            if(ex::is_format<material>(file_ext))
            {
                using asset_t = material;
                using entry_t = asset_handle<asset_t>;
                const auto& entry = am.find_asset_entry<asset_t>(relative);
                bool is_loading = !entry.is_ready();
                const auto& icon = tm.get_thumbnail(entry);
                bool selected = em.is_selected(entry);

                is_popup_opened |= draw_entry(
                    icon,
                    is_loading,
                    name,
                    absolute_path,
                    selected,
                    size,
                    [&]() // on_click
                    {
                        em.select(entry);
                    },
                    nullptr, // on_double_click
                    on_rename,
                    on_delete);
                return;
            }

            if(ex::is_format<mesh>(file_ext))
            {
                using asset_t = mesh;
                using entry_t = asset_handle<asset_t>;
                const auto& entry = am.find_asset_entry<asset_t>(relative);
                bool is_loading = !entry.is_ready();
                const auto& icon = tm.get_thumbnail(entry);
                bool selected = em.is_selected(entry);

                is_popup_opened |= draw_entry(
                    icon,
                    is_loading,
                    name,
                    absolute_path,
                    selected,
                    size,
                    [&]() // on_click
                    {
                        em.select(entry);
                    },
                    nullptr, // on_double_click
                    on_rename,
                    on_delete);
                return;
            }

            if(ex::is_format<animation>(file_ext))
            {
                using asset_t = animation;
                using entry_t = asset_handle<asset_t>;
                const auto& entry = am.find_asset_entry<asset_t>(relative);
                bool is_loading = !entry.is_ready();
                const auto& icon = tm.get_thumbnail(entry);
                bool selected = em.is_selected(entry);

                is_popup_opened |= draw_entry(
                    icon,
                    is_loading,
                    name,
                    absolute_path,
                    selected,
                    size,
                    [&]() // on_click
                    {
                        em.select(entry);
                    },
                    nullptr, // on_double_click
                    on_rename,
                    on_delete);
                return;
            }
        };

        ImGui::ItemBrowser(size, cache_.size(), [&](int index)
        {
            auto& cache_entry = cache_[index];
            process_cache_entry(cache_entry);
        });

        if(!is_popup_opened)
        {
            context_menu(ctx);
        }
        set_cache_path(current_path);

        ImGui::PopWindowFontSize();
    }
    ImGui::EndChild();
}

void content_browser::context_menu(rtti::context& ctx)
{
    if(ImGui::BeginPopupContextWindow())
    {
        context_create_menu(ctx);

        ImGui::Separator();

        if(ImGui::Selectable("OPEN IN ENVIRONMENT"))
        {
            fs::show_in_graphical_env(cache_.get_path());
        }

        ImGui::Separator();

        if(ImGui::Selectable("IMPORT..."))
        {
            import(ctx);
        }

        ImGui::EndPopup();
    }
}

void content_browser::context_create_menu(rtti::context& ctx)
{
    if(ImGui::BeginMenu("CREATE"))
    {
        if(ImGui::MenuItem("FOLDER"))
        {
            const auto available = get_new_file(cache_.get_path(), "New Folder");
            fs::error_code err;
            fs::create_directory(available, err);
        }

        ImGui::Separator();

        if(ImGui::MenuItem("MATERIAL"))
        {
            auto& am = ctx.get<asset_manager>();

            const auto available = get_new_file(cache_.get_path(), "New Material", ".mat");
            const auto key = fs::convert_to_protocol(available).generic_string();

            auto new_mat_future = am.load_asset_from_instance<material>(key, std::make_shared<standard_material>());
            asset_writer::save_to_file(new_mat_future.id(), new_mat_future);
        }

        ImGui::EndMenu();
    }
}

void content_browser::set_cache_path(const fs::path& path)
{
    if(cache_.get_path() == path)
    {
        return;
    }
    cache_.set_path(path);
    cache_path_with_protocol_ = fs::convert_to_protocol(path).generic_string();
}

void content_browser::import(rtti::context& ctx)
{
    std::vector<std::string> paths;
    if(native::open_files_dialog(paths, {}))
    {
        on_import(ctx, paths);
    }
}

void content_browser::on_import(rtti::context& ctx, const std::vector<std::string>& paths)
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
