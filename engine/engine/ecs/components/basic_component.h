#pragma once

#include "../ecs.h"

namespace ace
{

/**
 * @struct basic_component
 * @brief Basic component structure that other components can inherit from.
 */
struct basic_component
{
    bool eto{}; ///< Example boolean member variable.

    /**
     * @brief Marks the component as 'touched'.
     */
    void touch()
    {
    }
};

/**
 * @class owned_component
 * @brief Component that is owned by an entity.
 */
class owned_component : public basic_component
{
public:
    /**
     * @brief Sets the owner of the component.
     * @param owner The entity handle representing the owner.
     */
    void set_owner(entt::handle owner)
    {
        owner_ = owner;
    }

    /**
     * @brief Gets the owner of the component.
     * @return A constant handle to the owner entity.
     */
    [[nodiscard]] auto get_owner() const noexcept -> entt::const_handle
    {
        return owner_;
    }

    /**
     * @brief Gets the owner of the component.
     * @return A handle to the owner entity.
     */
    [[nodiscard]] auto get_owner() noexcept -> entt::handle
    {
        return owner_;
    }

private:
    entt::handle owner_{}; ///< The owner entity handle.
};

/**
 * @struct component_crtp
 * @brief CRTP (Curiously Recurring Template Pattern) base structure for components.
 * @tparam T The derived component type.
 * @tparam Base The base component type, defaults to basic_component.
 */
template<typename T, typename Base = basic_component>
struct component_crtp : Base
{
    static constexpr bool in_place_delete = true; ///< Indicates if the component can be deleted in place.

    using base = Base; ///< Type alias for the base component type.
};

} // namespace ace
