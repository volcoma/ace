#pragma once
#include "../../../imgui/integration/imgui.h"

#include <base/basetypes.hpp>
#include <context/context.hpp>
//#include <engine/assets/asset_manager.h>

namespace ace
{
class item_browser
{
public:
    using callback_t = std::function<void(int index)>;
    void draw(float item_width, size_t items_count, const callback_t& callback) const;

    template<typename T>
    void draw(rtti::context& ctx)
    {

    }

private:
    float scale_{1.0f};
};
} // namespace ace
