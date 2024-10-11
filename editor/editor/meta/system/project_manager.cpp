#include "project_manager.hpp"

#include <serialization/associative_archive.h>
#include <serialization/types/deque.hpp>
#include <serialization/types/string.hpp>

namespace ace
{
SAVE(project_manager::project)
{
    try_save(ar, ser20::make_nvp("path", obj.path));
}
SAVE_INSTANTIATE(project_manager::project, ser20::oarchive_associative_t);

LOAD(project_manager::project)
{
    try_load(ar, ser20::make_nvp("path", obj.path));
}
LOAD_INSTANTIATE(project_manager::project, ser20::iarchive_associative_t);

SAVE(project_manager::options)
{
    try_save(ar, ser20::make_nvp("recent_projects", obj.recent_projects));
}
SAVE_INSTANTIATE(project_manager::options, ser20::oarchive_associative_t);

LOAD(project_manager::options)
{
    try_load(ar, ser20::make_nvp("recent_projects", obj.recent_projects));
}
LOAD_INSTANTIATE(project_manager::options, ser20::iarchive_associative_t);
} // namespace ace
