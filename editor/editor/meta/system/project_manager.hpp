#pragma once

#include "../../system/project_manager.h"

#include <serialization/serialization.h>

namespace ace
{
SAVE_EXTERN(project_manager::project);
LOAD_EXTERN(project_manager::project);

SAVE_EXTERN(project_manager::options);
LOAD_EXTERN(project_manager::options);
} // namespace ace
