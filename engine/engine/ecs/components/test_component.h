#pragma once

#include <cstdint>
#include <string>
#include <engine/assets/asset_handle.h>
#include <engine/rendering/material.h>
#include <graphics/texture.h>
#include <math/math.h>
namespace ace
{

struct test_component
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

};

} // namespace ace
