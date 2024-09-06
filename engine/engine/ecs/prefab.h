#pragma once
#include <engine/engine_export.h>

#include <iosfwd>
#include <istream>
#include <memory>
#include <vector>
#include <filesystem/filesystem.h>

namespace ace
{

/**
 * @struct prefab
 * @brief Represents a generic prefab with a buffer for serialized data.
 */
struct prefab
{
    /**
     * @brief Buffer to store serialized data of the prefab.
     */
    fs::stream_buffer<std::vector<uint8_t>> buffer{};
};

/**
 * @struct scene_prefab
 * @brief Represents a scene-specific prefab.
 * Inherits from the generic prefab structure.
 */
struct scene_prefab : prefab
{
};

} // namespace ace
