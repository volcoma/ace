#pragma once

#include <string>
#include <uuid/uuid.h>

namespace ace
{

/**
 * @struct id_component
 * @brief Component that provides a unique identifier (UUID) for an entity.
 */
struct id_component
{
    /**
     * @brief The unique identifier for the entity.
     */
    hpp::uuid id = generate_uuid();
};

/**
 * @struct tag_component
 * @brief Component that provides a tag (name or label) for an entity.
 */
struct tag_component
{
    /**
     * @brief The tag or name of the entity.
     */
    std::string tag{};
};

} // namespace ace
