#pragma once

#include <base/basetypes.hpp>
#include <engine/assets/asset_handle.h>
#include <engine/rendering/gpu_program.h>
#include <engine/rendering/camera.h>
#include <hpp/optional.hpp>

namespace gfx
{
struct frame_buffer;
struct texture;
} // namespace gfx

namespace ace
{

class picking_manager
{
public:

    picking_manager();
    ~picking_manager();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void request_pick(math::vec2 pos, const camera& cam);
    constexpr static int tex_id_dim = 1;

    void on_frame_render(rtti::context& ctx, delta_t dt);

    void on_frame_pick(rtti::context& ctx, delta_t dt);
private:
    /// surface used to render into
    std::shared_ptr<gfx::frame_buffer> surface_;
    ///
    std::shared_ptr<gfx::texture> blit_tex_;
    /// picking program
    std::unique_ptr<gpu_program> program_;
    /// Read blit into this
    std::uint8_t blit_data_[tex_id_dim * tex_id_dim * 4];
    /// Indicates if is reading and when it will be ready
    std::uint32_t reading_ = 0;

    bool start_readback_ = false;

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

    hpp::optional<camera> pick_camera_{};
};
} // namespace ace
