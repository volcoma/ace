#include "program.hpp"
#include <engine/meta/assets/asset_handle.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>
#include <serialization/types/vector.hpp>

namespace ace
{
SAVE(gpu_program)
{
    try_save(ar, cereal::make_nvp("shaders", obj.get_shaders()));
}
SAVE_INSTANTIATE(gpu_program, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(gpu_program, cereal::oarchive_binary_t);

LOAD(gpu_program)
{
    std::vector<asset_handle<gfx::shader>> shaders;

    try_load(ar, cereal::make_nvp("shaders", shaders));

    for(const auto& shader : shaders)
    {
        obj.attach_shader(shader);
    }
    obj.populate();
}
LOAD_INSTANTIATE(gpu_program, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(gpu_program, cereal::iarchive_binary_t);
} // namespace ace
