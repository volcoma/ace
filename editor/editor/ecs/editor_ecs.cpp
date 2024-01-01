#include "editor_ecs.h"
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/components/id_component.h>
#include <engine/ecs/components/camera_component.h>

#include <logging/logging.h>

namespace ace
{

auto editor_ecs::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);
    return ecs::init(ctx);
}

auto editor_ecs::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);
    return ecs::deinit(ctx);
}


entt::handle editor_ecs::create_editor_camera()
{
    entt::handle ent(get_scene(), get_scene().create());
    ent.emplace<transform_component>().set_position_local({0.0f, 2.0f, -5.0f});
    ent.emplace<camera_component>();
    ent.emplace<entt::tag<"edit"_hs>>();
    editor_camera = ent;

    return ent;
}


} // namespace ace
