#include "audio_listener_component.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(audio_listener_component)
{
    rttr::registration::class_<audio_listener_component>("audio_listener_component")(
        rttr::metadata("category", "AUDIO"),
        rttr::metadata("pretty_name", "Audio Listener"))
        .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);
}

SAVE(audio_listener_component)
{
}
SAVE_INSTANTIATE(audio_listener_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(audio_listener_component, ser20::oarchive_binary_t);

LOAD(audio_listener_component)
{
}
LOAD_INSTANTIATE(audio_listener_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(audio_listener_component, ser20::iarchive_binary_t);
} // namespace ace
