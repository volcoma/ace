#pragma once


#include "audio_listener_component.hpp"
#include "audio_source_component.hpp"
#include "camera_component.hpp"
#include "id_component.hpp"
#include "light_component.hpp"
#include "model_component.hpp"
#include "animation_component.hpp"
#include "physics_component.hpp"
#include "prefab_component.hpp"
#include "reflection_probe_component.hpp"
#include "test_component.hpp"
#include "transform_component.hpp"
#include <tuple>

namespace ace
{

using all_serializeable_components = std::tuple<
    id_component,
    tag_component,
    prefab_component,
    transform_component,
    test_component,
    model_component,
    animation_component,
    bone_component,
    submesh_component,
    camera_component,
    light_component,
    skylight_component,
    reflection_probe_component,
    physics_component,
    audio_source_component,
    audio_listener_component
    >;

using all_inspectable_components = all_serializeable_components;

} // namespace ace
