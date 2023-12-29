#pragma once

#include "basic_component.h"


#include <engine/assets/asset_handle.h>


namespace ace
{

struct prefab_component : component_crtp<prefab_component>
{
    asset_handle<prefab> source;
};

} // namespace ace
