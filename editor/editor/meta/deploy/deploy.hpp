#pragma once
#include <editor/editing/editor_actions.h>
#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(deploy_params);
SAVE_EXTERN(deploy_params);
LOAD_EXTERN(deploy_params);
}
