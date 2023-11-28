#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <memory>


namespace ace
{
class gpu_program;
class debugdraw_system
{
public:
	debugdraw_system();
	~debugdraw_system();


    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;


    void on_frame_render(rtti::context& ctx, delta_t dt);

private:
	///
	std::unique_ptr<gpu_program> program_;
};
}
