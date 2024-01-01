#include "game_panel.h"

#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/ecs.h>

namespace ace
{

void game_panel::draw_menubar(rtti::context& ctx)
{
    if(ImGui::BeginMenuBar())
    {
        ImGui::EndMenuBar();
    }
}

void game_panel::init(rtti::context& ctx)
{
}

void game_panel::draw(rtti::context& ctx)
{
    draw_menubar(ctx);

    auto& ec = ctx.get<ecs>();
    auto size = ImGui::GetContentRegionAvail();

    if(size.x > 0 && size.y > 0)
    {
        bool rendered = false;
        ec.get_scene().view<camera_component>().each(
            [&](auto e, auto&& camera_comp)
            {
                camera_comp.set_viewport_size({static_cast<std::uint32_t>(size.x), static_cast<std::uint32_t>(size.y)});

                const auto& camera = camera_comp.get_camera();
                auto& render_view = camera_comp.get_render_view();
                const auto& viewport_size = camera.get_viewport_size();
                const auto surface = render_view.get_output_fbo(viewport_size);
                auto tex = surface->get_attachment(0).texture;
                ImGui::Image(ImGui::ToId(tex), size);
                rendered = true;
            });

        if(!rendered)
        {
            static const auto text = "No cameras rendering";
            ImGui::SetCursorPosY(size.y * 0.5f);
            ImGui::AlignedItem(0.5f,
                               size.x,
                               ImGui::CalcTextSize(text).x,
                               []()
                               {
                                   ImGui::TextUnformatted(text);
                               });
        }
    }
}

} // namespace ace
