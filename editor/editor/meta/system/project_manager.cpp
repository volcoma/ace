#include "project_manager.hpp"

#include <serialization/associative_archive.h>
#include <serialization/types/deque.hpp>
#include <serialization/types/string.hpp>

namespace ace
{
SAVE(project_manager::options)
{
    try_save(ar, cereal::make_nvp("recent_projects", obj.recent_projects));
}
SAVE_INSTANTIATE(project_manager::options, cereal::oarchive_associative_t);

LOAD(project_manager::options)
{
    try_load(ar, cereal::make_nvp("recent_projects", obj.recent_projects));
}
LOAD_INSTANTIATE(project_manager::options, cereal::iarchive_associative_t);
} // namespace ace
