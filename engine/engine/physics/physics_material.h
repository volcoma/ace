#pragma once
#include <memory>
namespace ace
{

struct physics_material
{
    using sptr = std::shared_ptr<physics_material>;
    using wptr = std::weak_ptr<physics_material>;
    using uptr = std::unique_ptr<physics_material>;

    float restitution{0};
    float friction{0.5};
    float spin_friction{0};
    float roll_friction{0};
    float stiffness{1e18};
    float damping{0.1f};
};

} // namespace ace
