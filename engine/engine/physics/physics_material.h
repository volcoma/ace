#pragma once
#include <memory>

#include <type_traits>


namespace ace
{

struct physics_material
{
    using sptr = std::shared_ptr<physics_material>;
    using wptr = std::weak_ptr<physics_material>;
    using uptr = std::unique_ptr<physics_material>;

    float restitution{0};            // Range: 0.0 to 1.0
    // Tooltip: Coefficient of restitution. Defines the bounciness of the material. A value of 0 means no bounce, while 1 means perfect bounce.

    float friction{0.5};             // Range: 0.0 to 1.0 (sometimes slightly above 1.0)
    // Tooltip: Coefficient of friction. Determines the resistance to sliding motion. Typical values range from 0 (no friction) to 1 (high friction).

    float spin_friction{0};          // Range: 0.0 to 1.0
    // Tooltip: Coefficient of spin friction. Defines resistance to spinning. Similar to friction but applies to rotational motion.

    float roll_friction{0};          // Range: 0.0 to 1.0
    // Tooltip: Coefficient of rolling friction. Determines resistance to rolling motion. Usually lower than sliding friction.

    float stiffness{0.5};            // Range: 0.0 to 1.0 (user-friendly range)
    // Tooltip: Normalized stiffness value. Represents the elasticity of the material. Higher values indicate stiffer materials.

    float damping{0.1f};             // Range: 0.0 to 1.0 (user-friendly range)
    // Tooltip: Normalized damping coefficient. Represents the material's resistance to motion. Higher values result in more energy loss.


    // Convert normalized stiffness to actual stiffness
    auto get_stiffness() const -> float
    {
        const float min_stiffness = 1e3f;  // Minimum actual stiffness
        const float max_stiffness = 1e5f;  // Maximum actual stiffness
        return min_stiffness + stiffness * (max_stiffness - min_stiffness);
    }

    // Convert normalized damping to actual damping
    auto get_damping() const -> float
    {
        const float min_damping = 0.01f;  // Minimum actual damping
        const float max_damping = 0.3f;   // Maximum actual damping
        return min_damping + damping * (max_damping - min_damping);
    }
};

} // namespace ace
