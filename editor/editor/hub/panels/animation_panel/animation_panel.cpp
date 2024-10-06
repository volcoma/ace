#include "animation_panel.h"
#include "../panels_defs.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui_widgets/gizmo.h>


namespace ace
{
namespace
{

} // namespace

void animation_panel::draw_menubar(rtti::context& ctx)
{ 
    if(ImGui::BeginMenuBar())
    {

        ImGui::EndMenuBar();
    }
}

animation_panel::animation_panel(imgui_panels* parent)
{
}

void animation_panel::init(rtti::context& ctx)
{
    struct CustomNode : public ImFlow::BaseNode
    {
        explicit CustomNode()
        {
            setTitle("Custom");
            setStyle(ImFlow::NodeStyle::brown());
            addIN<int>("InTest", "int", 0, ImFlow::ConnectionFilter::SameType(), ImFlow::PinStyle::red());

            addOUT<int>("OutTest", "int", ImFlow::PinStyle::blue())
                ->behaviour(
                    [this]()
                    {
                        return 0;
                    });
        }
    };


    struct Custom2Node : public ImFlow::BaseNode
    {
        explicit Custom2Node()
        {
            setTitle("Custom2");
            setStyle(ImFlow::NodeStyle::brown());
            addIN<int>("In1Test", "int", 0, ImFlow::ConnectionFilter::SameType(), ImFlow::PinStyle::red());
            addIN<float>("In2Test", "float", 0, ImFlow::ConnectionFilter::SameType(), ImFlow::PinStyle::red());

            addOUT<int>("Out1Test", "int", ImFlow::PinStyle::blue())
                ->behaviour(
                    [this]()
                    {
                        return 0;
                    });
            addOUT<float>("Out2Test", "float", ImFlow::PinStyle::blue())
                ->behaviour(
                    [this]()
                    {
                        return 0.0f;
                    });

        }


        void draw() override
        {
            ImGui::PushFont(ImGui::Font::Bold);
            ImGui::Text("%s", "some text here");

            ImGui::PopFont();
        }
    };

    auto callback = [this]()
    {
        /* omitted */

        float size = 200.0f;
        static ImGuiTextFilter filter_;

        ImGui::DrawFilterWithHint(filter_, ICON_MDI_SELECT_SEARCH " Search...", size);
        ImGui::DrawItemActivityOutline();

        ImGui::Separator();
        ImGui::BeginChild("COMPONENT_MENU_CONTEXT", ImVec2(ImGui::GetContentRegionAvail().x, size));

        struct node_factory
        {
            std::string name;
            std::function<void()> factory;
        };

        std::vector<node_factory> nodes{
            {"Custom",
             [this]()
             {
                 flow_.placeNode<CustomNode>();
             }},
            {"Custom2",
             [this]()
             {
                 flow_.placeNode<Custom2Node>();
             }},
        };

        for(const auto& factory : nodes)
        {
            if(!filter_.PassFilter(factory.name.c_str()))
                continue;

            if(ImGui::Selectable(factory.name.c_str()))
            {
                factory.factory();

                ImGui::CloseCurrentPopup();
            }
        };

        ImGui::EndChild();
    };

    flow_.rightClickPopUpContent(
        [this, callback](ImFlow::BaseNode* node)
        {
            callback();
        });

    flow_.droppedLinkPopUpContent(
        [this, callback](ImFlow::Pin* dragged)
        {
            callback();
        });

    flow_.addNode<CustomNode>({});
}

void animation_panel::deinit(rtti::context& ctx)
{

}

void animation_panel::on_frame_update(rtti::context& ctx, delta_t dt)
{

}


void animation_panel::on_frame_render(rtti::context& ctx, delta_t dt)
{

}

void animation_panel::on_frame_ui_render(rtti::context& ctx, const char* name)
{
    if(ImGui::Begin(name, nullptr, ImGuiWindowFlags_MenuBar))
    {
        // ImGui::WindowTimeBlock block(ImGui::GetFont(ImGui::Font::Mono));

        set_visible(true);
        draw_ui(ctx);

    }
    else
    {
        set_visible(false);
    }
    ImGui::End();
}


void animation_panel::set_visible(bool visible)
{
    is_visible_ = visible;
}

void animation_panel::draw_ui(rtti::context& ctx)
{
    flow_.update();

}

} // namespace ace
