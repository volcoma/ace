#include "audio_listener_component.h"

namespace ace
{
void audio_listener_component::update(const math::transform& t, delta_t dt)
{
    if(!listener_)
    {
        listener_ = std::make_shared<audio::listener>();
    }

    auto pos = t.get_position();
    auto forward = t.z_unit_axis();
    auto up = t.y_unit_axis();
    listener_->set_position({{pos.x, pos.y, pos.z}});
    listener_->set_orientation({{forward.x, forward.y, forward.z}}, {{up.x, up.y, up.z}});
}

} // namespace ace
