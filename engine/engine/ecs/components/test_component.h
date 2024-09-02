#pragma once

#include "basic_component.h"

#include <cstdint>
#include <engine/assets/asset_handle.h>
#include <engine/rendering/material.h>
#include <engine/animation/animation.h>

#include <graphics/texture.h>
#include <math/math.h>
#include <string>

namespace ace
{

struct test_component : public component_crtp<test_component>
{
    std::string str{};
    uint8_t u8{};
    uint16_t u16{};
    uint32_t u32{};
    uint64_t u64{};

    int8_t i8{};
    int16_t i16{};
    int32_t i32{};
    int64_t i64{};

    float f{};
    double d{};

    irange32_t irange{};
    isize32_t isize{};
    ipoint32_t ipoint{};
    irect32_t irect{};

    delta_t delta{};

    math::color color{};

    asset_handle<gfx::texture> texture;
    asset_handle<material> mat;
    asset_handle<animation_clip> anim;
};

} // namespace ace
