#pragma once

#include "basic_component.h"
#include <engine/assets/asset_handle.h>

namespace ace
{

/**
 * @struct prefab_component
 * @brief Component that holds a reference to a prefab asset.
 */
struct prefab_component : public component_crtp<prefab_component>
{
    /**
     * @brief Handle to the prefab asset.
     */
    asset_handle<prefab> source;
};

} // namespace ace
