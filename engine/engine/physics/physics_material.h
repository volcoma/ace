#pragma once

namespace ace
{

struct physics_material
{
    float restitution{0};
    float friction{0.5};
    float spin_friction{0};
    float roll_friction{0};
    float stiffness{1e18};
    float damping{1e18};
};

} // namespace ace
