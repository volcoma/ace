#pragma once

#include "basic_component.h"
#include <math/math.h>

namespace ace
{

class box_collider_component : public component_crtp<box_collider_component, owned_component>
{
public:
    static void on_create_component(entt::registry& r, const entt::entity e);
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    void set_extends(const math::vec3& extends);
    auto get_extends() const noexcept -> const math::vec3&;

    void on_phyiscs_simulation_begin();
    void on_phyiscs_simulation_end();

    void recreate_phyisics_shape();
private:
    void on_change_extends();
    void destroy_phyisics_shape();


    math::vec3 extends_{1.0f, 1.0f, 1.0f};
};
} // namespace ace
