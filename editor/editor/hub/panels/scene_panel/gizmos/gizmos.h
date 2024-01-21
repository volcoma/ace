#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <engine/ecs/ecs.h>
#include <engine/assets/asset_manager.h>
#include <memory>


namespace ace
{
class camera;
class gpu_program;
class debugdraw_rendering
{
public:
	debugdraw_rendering();
	~debugdraw_rendering();


    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;


    void on_frame_render(rtti::context& ctx, entt::handle camera_entity);

private:
    void draw_grid(uint32_t pass_id, const camera& cam, float opacity);
    void draw_shapes(asset_manager& am, uint32_t pass_id, const camera& cam, entt::handle selected);
	///
	std::unique_ptr<gpu_program> wireframe_program_;
    std::unique_ptr<gpu_program> grid_program_;

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
}
