#pragma once

#include "../ecs.h"

namespace ace
{

struct basic_component
{
    bool eto{};

    void touch()
    {
    }
};

class owned_component : public basic_component
{
public:
    void set_owner(entt::handle owner)
    {
        owner_ = owner;
    }
    [[nodiscard]] auto get_owner() const noexcept -> entt::const_handle
    {
        return owner_;
    }

    [[nodiscard]] auto get_owner() noexcept -> entt::handle
    {
        return owner_;
    }

private:
    entt::handle owner_{};
};

template<typename T, typename Base = basic_component>
struct component_crtp : Base
{
    static constexpr bool in_place_delete = true;

    using base = Base;
};

} // namespace ace
