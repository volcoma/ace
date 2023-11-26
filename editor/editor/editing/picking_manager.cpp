#include "picking_manager.h"
#include "editing_manager.h"

#include <graphics/render_pass.h>
#include <graphics/texture.h>
#include <logging/logging.h>

#include <engine/assets/asset_manager.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/events.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/rendering/render_window.h>
#include <engine/rendering/renderer.h>

namespace ace
{
constexpr int picking_manager::tex_id_dim;

void picking_manager::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& ec = ctx.get<ecs>();
    auto& rend = ctx.get<renderer>();
    auto& em = ctx.get<editing_manager>();
    //	auto& input = core::get_subsystem<runtime::input>();
    //	auto& renderer = core::get_subsystem<runtime::renderer>();
    //	auto& ecs = core::get_subsystem<runtime::entity_component_system>();

    const auto render_frame = rend.get_render_frame();

    if(pick_pos_)
    {
        auto& editor_camera = ec.editor_camera;
        //		if(imguizmo::is_over() && em.selection_data.object)
        //			return;

        if(!editor_camera)
            return;

        auto& camera_comp = editor_camera.get<camera_component>();
        const auto& current_camera = camera_comp.get_camera();
        const auto near_clip = current_camera.get_near_clip();
        const auto far_clip = current_camera.get_far_clip();
        const auto& mouse_pos = *pick_pos_;
        const auto& frustum = current_camera.get_frustum();
        math::vec2 cursor_pos = math::vec2{mouse_pos.x, mouse_pos.y};
        math::vec3 pick_eye;
        math::vec3 pick_at;
        math::vec3 pick_up = {0.0f, 1.0f, 0.0f};

        pick_pos_.reset();

        if(!current_camera.viewport_to_world(cursor_pos,
                                             frustum.planes[math::volume_plane::near_plane],
                                             pick_eye,
                                             true))
            return;

        if(!current_camera.viewport_to_world(cursor_pos, frustum.planes[math::volume_plane::far_plane], pick_at, true))
            return;

        reading_ = 0;
        start_readback_ = true;

        camera pick_camera;
        pick_camera.set_aspect_ratio(1.0f);
        pick_camera.set_fov(1.0f);
        pick_camera.set_near_clip(near_clip);
        pick_camera.set_far_clip(far_clip);
        pick_camera.look_at(pick_eye, pick_at, pick_up);

        const auto& pick_view = pick_camera.get_view();
        const auto& pick_proj = pick_camera.get_projection();
        const auto& pick_frustum = pick_camera.get_frustum();

        gfx::render_pass pass("picking_buffer_fill");
        // ID buffer clears to black, which represents clicking on nothing (background)
        pass.clear(BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000ff, 1.0f, 0);
        pass.set_view_proj(pick_view, pick_proj);
        pass.bind(surface_.get());

        ec.registry.view<transform_component, model_component>().each(
            [&](auto e, auto&& transform_comp, auto&& model_comp)
            {
                auto& model = model_comp.get_model();
                if(!model.is_valid())
                    return;

                const auto& world_transform = transform_comp.get_transform_global();

                auto lod = model.get_lod(0);
                if(!lod)
                    return;

                const auto& mesh = lod.get();
                const auto& bounds = mesh.get_bounds();

                // Test the bounding box of the mesh
                if(!math::frustum::test_obb(pick_frustum, bounds, world_transform))
                    return;

                auto id = ENTT_ID_TYPE(e);
                std::uint32_t rr = (id)&0xff;
                std::uint32_t gg = (id >> 8) & 0xff;
                std::uint32_t bb = (id >> 16) & 0xff;
                math::vec4 color_id = {rr / 255.0f, gg / 255.0f, bb / 255.0f, 1.0f};

                const auto& bone_transforms = model_comp.get_bone_transforms();
                model.render(pass.id,
                             world_transform,
                             bone_transforms,
                             true,
                             true,
                             true,
                             0,
                             0,
                             program_.get(),
                             [&](auto& p)
                             {
                                 p.set_uniform("u_id", &color_id);
                             });
            });
    }

    // If the user previously clicked, and we're done reading data from GPU, look at ID buffer on CPU
    // Whatever mesh has the most pixels in the ID buffer is the one the user clicked on.
    if((reading_ == 0u) && start_readback_)
    {
        bool blit_support = gfx::is_supported(BGFX_CAPS_TEXTURE_BLIT);

        if(blit_support == false)
        {
            APPLOG_WARNING("Texture blitting is not supported. Picking will not work");
            start_readback_ = false;
            return;
        }

        gfx::render_pass pass("picking_buffer_blit");
        pass.touch();
        // Blit and read
        gfx::blit(pass.id, blit_tex_->native_handle(), 0, 0, surface_->get_texture()->native_handle());
        reading_ = gfx::read_texture(blit_tex_->native_handle(), blit_data_);
        start_readback_ = false;
    }

    if(reading_ && reading_ <= render_frame)
    {
        reading_ = 0;
        std::map<std::uint32_t, std::uint32_t> ids; // This contains all the IDs found in the buffer
        std::uint32_t max_amount = 0;
        for(std::uint8_t* x = blit_data_; x < blit_data_ + tex_id_dim * tex_id_dim * 4;)
        {
            std::uint8_t rr = *x++;
            std::uint8_t gg = *x++;
            std::uint8_t bb = *x++;
            std::uint8_t aa = *x++;
            (void)aa;
            if(gfx::renderer_type::Direct3D9 == gfx::get_renderer_type())
            {
                // Comes back as BGRA
                std::swap(rr, bb);
            }

            // Skip background
            if(0 == (rr | gg | bb))
            {
                continue;
            }

            auto hash_key = static_cast<std::uint32_t>(rr + (gg << 8) + (bb << 16));
            std::uint32_t amount = 1;
            auto mapIter = ids.find(hash_key);
            if(mapIter != ids.end())
            {
                amount = mapIter->second + 1;
            }

            // Amount of times this ID (color) has been clicked on in buffer
            ids[hash_key] = amount;
            max_amount = max_amount > amount ? max_amount : amount;
        }

        ENTT_ID_TYPE id_key = 0;
        if(max_amount != 0u)
        {
            for(auto& pair : ids)
            {
                if(pair.second == max_amount)
                {
                    id_key = pair.first;

                    auto entity = entt::entity(id_key);
                    entt::handle picked_entity(ec.registry, entity);
                    if(picked_entity)
                    {
                        em.select(picked_entity);
                    }
                    break;
                }
            }
        }
        else
        {
            em.unselect();
        }
    }
}

picking_manager::picking_manager()
{
}

picking_manager::~picking_manager()
{
}

auto picking_manager::init(rtti::context& ctx) -> bool
{
    auto& ev = ctx.get<events>();
    ev.on_frame_render.connect(sentinel_, this, &picking_manager::on_frame_render);

    auto& am = ctx.get<asset_manager>();

    // Set up ID buffer, which has a color target and depth buffer
    auto picking_rt =
        std::make_shared<gfx::texture>(tex_id_dim,
                                       tex_id_dim,
                                       false,
                                       1,
                                       gfx::texture_format::RGBA8,
                                       0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
                                           BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

    auto picking_rt_depth =
        std::make_shared<gfx::texture>(tex_id_dim,
                                       tex_id_dim,
                                       false,
                                       1,
                                       gfx::texture_format::D24S8,
                                       0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
                                           BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

    std::vector<std::shared_ptr<gfx::texture>> textures{picking_rt, picking_rt_depth};
    surface_ = std::make_shared<gfx::frame_buffer>(textures);

    // CPU texture for blitting to and reading ID buffer so we can see what was clicked on.
    // Impossible to read directly from a render target, you *must* blit to a CPU texture
    // first. Algorithm Overview: Render on GPU -> Blit to CPU texture -> Read from CPU
    // texture.
    blit_tex_ = std::make_shared<gfx::texture>(
        tex_id_dim,
        tex_id_dim,
        false,
        1,
        gfx::texture_format::RGBA8,
        0 | BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
            BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);

    auto vs = am.load<gfx::shader>("editor:/data/shaders/vs_picking_id.sc");
    auto fs = am.load<gfx::shader>("editor:/data/shaders/fs_picking_id.sc");

    program_ = std::make_unique<gpu_program>(vs, fs);

    return true;
}

auto picking_manager::deinit(rtti::context& ctx) -> bool
{
    return true;
}

void picking_manager::request_pick(glm::vec2 pos)
{
    pick_pos_ = pos;
}
} // namespace ace
