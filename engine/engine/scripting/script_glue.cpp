#include "script_glue.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>

#include <engine/engine.h>
#include <monopp/mono_exception.h>
#include <monopp/mono_internal_call.h>
#include <monopp/mono_jit.h>
#include <monort/monort.h>

#include <filesystem/filesystem.h>
#include <logging/logging.h>

namespace ace
{
namespace
{
void Internal_CreateScene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
    // std::cout << "FROM C++ : MyObject created." << std::endl;
}

void Internal_DestroyScene(const mono::mono_object& this_ptr)
{
    mono::ignore(this_ptr);
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;
}

uint32_t Internal_CreateEntity(const std::string& tag)
{
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;

    auto& ctx = engine::context();
    auto& ec = ctx.get<ecs>();
    auto e = ec.get_scene().create_entity(tag);
    return static_cast<uint32_t>(e.entity());
}

bool Internal_DestroyEntity(uint32_t id)
{
    // std::cout << "FROM C++ : MyObject deleted." << std::endl;

    return true;
}

bool Internal_IsEntityValid(uint32_t id)
{
    return true;
}

void Internal_LogTrace(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_TRACE_LOC(file.c_str(), line, func.c_str(), message);

}

void Internal_LogInfo(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_INFO_LOC(file.c_str(), line, func.c_str(), message);
}

void Internal_LogWarning(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_WARNING_LOC(file.c_str(), line, func.c_str(), message);

}

void Internal_LogError(const std::string& message, const std::string& func, const std::string& file, int line)
{
    APPLOG_ERROR_LOC(file.c_str(), line, func.c_str(), message);
}

} // namespace

auto script_glue::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    mono::add_internal_call("Ace.Core.Logger::Internal_LogTrace", internal_vcall(Internal_LogTrace));
    mono::add_internal_call("Ace.Core.Logger::Internal_LogInfo", internal_vcall(Internal_LogInfo));
    mono::add_internal_call("Ace.Core.Logger::Internal_LogWarning", internal_vcall(Internal_LogWarning));
    mono::add_internal_call("Ace.Core.Logger::Internal_LogError", internal_vcall(Internal_LogError));

    mono::add_internal_call("Ace.Core.Scene::Internal_CreateScene", internal_vcall(Internal_CreateScene));
    mono::add_internal_call("Ace.Core.Scene::Internal_DestroyScene", internal_vcall(Internal_DestroyScene));
    mono::add_internal_call("Ace.Core.Scene::Internal_CreateEntity", internal_rcall(Internal_CreateEntity));
    mono::add_internal_call("Ace.Core.Scene::Internal_DestroyEntity", internal_rcall(Internal_DestroyEntity));
    mono::add_internal_call("Ace.Core.Scene::Internal_IsEntityValid", internal_rcall(Internal_IsEntityValid));
    // mono::managed_interface::init(assembly);

    return true;
}

auto script_glue::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}

} // namespace ace
