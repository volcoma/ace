#include "test_component.hpp"
#include <engine/meta/assets/asset_handle.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(test_component)
{
    rttr::registration::class_<test_component>("test_component")(rttr::metadata("category", "BASIC"),
                                                                 rttr::metadata("pretty_name", "Test"))
        .constructor<>()()
        .property("str", &test_component::str)
        .property("u8", &test_component::u8)
        .property("u8[restricted]", &test_component::u8)(rttr::metadata("min", 12), rttr::metadata("max", 200))
        .property("u16", &test_component::u16)
        .property("u32", &test_component::u32)
        .property("u64", &test_component::u64)
        .property("i8", &test_component::i8)
        .property("i16", &test_component::i16)
        .property("i32", &test_component::i32)
        .property("i64", &test_component::i64)
        .property("f", &test_component::f)
        .property("d", &test_component::d)
        .property("d[restricted]", &test_component::d)(rttr::metadata("min", 12.0), rttr::metadata("max", 200.0))
        .property("irange", &test_component::irange)
        .property("isize", &test_component::isize)
        .property("ipoint", &test_component::ipoint)
        .property("irect", &test_component::irect)
        .property("delta", &test_component::delta)
        .property("color", &test_component::color)
        .property("texture", &test_component::texture)
        .property("mat", &test_component::mat)
        .property("anim", &test_component::anim);
}

SAVE(test_component)
{
}
SAVE_INSTANTIATE(test_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(test_component, cereal::oarchive_binary_t);

LOAD(test_component)
{
}
LOAD_INSTANTIATE(test_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(test_component, cereal::iarchive_binary_t);

} // namespace ace
