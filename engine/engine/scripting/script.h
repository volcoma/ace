#pragma once
#include <engine/engine_export.h>

#include <chrono>
#include <string>
#include <vector>

namespace ace
{

struct script
{
    using sptr = std::shared_ptr<script>; ///< Shared pointer to a physics material.
    using wptr = std::weak_ptr<script>;   ///< Weak pointer to a physics material.
    using uptr = std::unique_ptr<script>; ///< Unique pointer to a physics material.

};

} // namespace ace
