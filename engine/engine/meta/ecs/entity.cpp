#include "entity.hpp"

#include <chrono>
#include <serialization/archives/yaml.hpp>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include "components/all_components.h"

#include "entt/entity/fwd.hpp"
#include "logging/logging.h"

#include <hpp/utility.hpp>
#include <sstream>

namespace ace
{

auto const_handle_cast(entt::const_handle chandle) -> entt::handle
{
    entt::handle handle(*const_cast<entt::handle::registry_type*>(chandle.registry()), chandle.entity());
    return handle;
}

struct entity_loader
{
    entt::registry* reg{};
    std::map<entt::entity, entt::handle> mapping;
};

template<typename Entity>
struct entity_components
{
    Entity entity;
};

template<typename Entity>
struct entity_data
{
    entity_components<Entity> components;
};

thread_local entity_loader* current_loader{};

void set_loader(entity_loader& loader)
{
    current_loader = &loader;
}

void reset_loader()
{
    current_loader = {};
}

auto get_loader() -> entity_loader&
{
    assert(current_loader);
    return *current_loader;
}

} // namespace ace

using namespace ace;

namespace ser20
{

SAVE(entt::const_handle)
{
    try_save(ar, ser20::make_nvp("id", obj.entity()));
}
SAVE_INSTANTIATE(entt::const_handle, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(entt::const_handle, ser20::oarchive_binary_t);

LOAD(entt::handle)
{
    entt::handle::entity_type id{};
    try_load(ar, ser20::make_nvp("id", id));

    if(id != entt::null)
    {
        auto& loader = get_loader();
        auto it = loader.mapping.find(id);
        if(it != loader.mapping.end())
        {
            obj = it->second;
        }
        else if(obj)
        {
            loader.mapping[id] = obj;
        }
        else
        {
            entt::handle h(*loader.reg, loader.reg->create());
            loader.mapping[id] = h;
            obj = h;
        }
    }
}

LOAD_INSTANTIATE(entt::handle, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(entt::handle, ser20::iarchive_binary_t);

SAVE(entity_components<entt::const_handle>)
{
    hpp::for_each_tuple_type<ace::all_serializeable_components>(
        [&](auto index)
        {
            using ctype = std::tuple_element_t<decltype(index)::value, ace::all_serializeable_components>;
            auto component = obj.entity.try_get<ctype>();

            auto name = rttr::get_pretty_name(rttr::type::get<ctype>());

            auto has_name = "Has" + name;
            try_save(ar, ser20::make_nvp(has_name, component != nullptr));

            if(component)
            {
                try_save(ar, ser20::make_nvp(name, *component));
            }
        });
}
SAVE_INSTANTIATE(entity_components<entt::const_handle>, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(entity_components<entt::const_handle>, ser20::oarchive_binary_t);

LOAD(entity_components<entt::handle>)
{
    hpp::for_each_tuple_type<ace::all_serializeable_components>(
        [&](auto index)
        {
            using ctype = std::tuple_element_t<decltype(index)::value, ace::all_serializeable_components>;

            auto component_type = rttr::type::get<ctype>();
            std::string name = component_type.get_name().data();
            auto meta_id = component_type.get_metadata("pretty_name");
            if(meta_id)
            {
                name = meta_id.to_string();
            }

            auto has_name = "Has" + name;
            bool has_component = false;
            try_load(ar, ser20::make_nvp(has_name, has_component));

            if(has_component)
            {
                auto& component = obj.entity.emplace_or_replace<ctype>();
                try_load(ar, ser20::make_nvp(name, component));
            }
        });
}
LOAD_INSTANTIATE(entity_components<entt::handle>, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(entity_components<entt::handle>, ser20::iarchive_binary_t);

SAVE(entity_data<entt::const_handle>)
{
    SAVE_FUNCTION_NAME(ar, obj.components.entity);
    try_save(ar, ser20::make_nvp("components", obj.components));
}
SAVE_INSTANTIATE(entity_data<entt::const_handle>, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(entity_data<entt::const_handle>, ser20::oarchive_binary_t);

LOAD(entity_data<entt::handle>)
{
    entt::handle e;
    LOAD_FUNCTION_NAME(ar, e);

    obj.components.entity = e;
    try_load(ar, ser20::make_nvp("components", obj.components));
}
LOAD_INSTANTIATE(entity_data<entt::handle>, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(entity_data<entt::handle>, ser20::iarchive_binary_t);

} // namespace ser20

namespace ace
{
namespace
{

void flatten_hierarchy(entt::const_handle obj, std::vector<entity_data<entt::const_handle>>& entities)
{
    auto& trans_comp = obj.get<transform_component>();
    const auto& children = trans_comp.get_children();

    entity_data<entt::const_handle> data;
    data.components.entity = obj;

    entities.emplace_back(std::move(data));

    entities.reserve(entities.size() + children.size());
    for(const auto& child : children)
    {
        flatten_hierarchy(child, entities);
    }
}

template<typename Archive>
void save_to_archive(Archive& ar, entt::const_handle obj)
{
    bool is_root = obj.all_of<root_component>();
    if(!is_root)
    {
        const_handle_cast(obj).emplace<root_component>();
    }

    auto& trans_comp = obj.get<transform_component>();

    std::vector<entity_data<entt::const_handle>> entities;
    flatten_hierarchy(obj, entities);

    try_save(ar, ser20::make_nvp("entities", entities));

    static const std::string version = "1.0.0";
    try_save(ar, ser20::make_nvp("version", version));

    if(!is_root)
    {
        const_handle_cast(obj).erase<root_component>();
    }
}

template<typename Archive>
auto load_from_archive_impl(Archive& ar, entt::registry& registry, const std::function<void(entt::handle)>& on_create)
    -> entt::handle
{
    std::vector<entity_data<entt::handle>> entities;
    try_load(ar, ser20::make_nvp("entities", entities));

    std::string version;
    try_load(ar, ser20::make_nvp("version", version));

    entt::handle result{};
    if(!entities.empty())
    {
        result = entities.front().components.entity;
    }

    if(on_create)
    {
        for(const auto& e : entities)
        {
            on_create(e.components.entity);
        }
    }

    return result;
}

template<typename Archive>
auto load_from_archive_start(Archive& ar,
                             entt::registry& registry,
                             const std::function<void(entt::handle)>& on_create = {}) -> entt::handle
{
    entity_loader loader;
    loader.reg = &registry;

    set_loader(loader);

    auto obj = load_from_archive_impl(ar, registry, on_create);

    reset_loader();

    return obj;
}

template<typename Archive>
void load_from_archive(Archive& ar, entt::handle& obj, const std::function<void(entt::handle)>& on_create = {})
{
    obj = load_from_archive_start(ar, *obj.registry(), on_create);
}

template<typename Archive>
void save_to_archive(Archive& ar, const entt::registry& reg)
{
    size_t count = 0;
    reg.view<transform_component, root_component>().each(
        [&](auto e, auto&& comp1, auto&& comp2)
        {
            count++;
        });

    try_save(ar, ser20::make_nvp("entities_count", count));
    reg.view<transform_component, root_component>().each(
        [&](auto e, auto&& comp1, auto&& comp2)
        {
            save_to_archive(ar, entt::const_handle(reg, e));
        });
}

template<typename Archive>
void load_from_archive(Archive& ar, entt::registry& reg)
{
    reg.clear();
    size_t count = 0;
    try_load(ar, ser20::make_nvp("entities_count", count));

    for(size_t i = 0; i < count; ++i)
    {
        entt::handle e(reg, reg.create());
        load_from_archive(ar, e);
    }
}

} // namespace

void save_to_stream(std::ostream& stream, entt::const_handle obj)
{
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_oarchive_associative(stream);
        save_to_archive(ar, obj);
    }
}

void save_to_file(const std::string& absolute_path, entt::const_handle obj)
{
    std::ofstream stream(absolute_path);

    save_to_stream(stream, obj);
}

void save_to_stream_bin(std::ostream& stream, entt::const_handle obj)
{
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);

        save_to_archive(ar, obj);
    }
}

void save_to_file_bin(const std::string& absolute_path, entt::const_handle obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    save_to_stream_bin(stream, obj);
}

void load_from_view(std::string_view view, entt::handle& obj)
{
    if(!view.empty())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(view.data(), view.size());
        load_from_archive(ar, obj);
    }
}

void load_from_stream(std::istream& stream, entt::handle& obj)
{
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(stream);
        load_from_archive(ar, obj);
    }
}

void load_from_file(const std::string& absolute_path, entt::handle& obj)
{
    std::ifstream stream(absolute_path);
    load_from_stream(stream, obj);
}

void load_from_stream_bin(std::istream& stream, entt::handle& obj)
{
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        ser20::iarchive_binary_t ar(stream);
        load_from_archive(ar, obj);
    }
}

void load_from_file_bin(const std::string& absolute_path, entt::handle& obj)
{
    APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ifstream stream(absolute_path, std::ios::binary);
    load_from_stream_bin(stream, obj);
}

auto load_from_prefab(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle
{
    entt::handle obj;

    const auto& prefab = pfb.get();
    const auto& buffer = prefab->buffer.data;

    if(!buffer.empty())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(buffer.data(), buffer.size());

        auto on_create = [&pfb](entt::handle obj)
        {
            if(obj)
            {
                auto& pfb_comp = obj.get_or_emplace<prefab_component>();
                pfb_comp.source = pfb;
            }
        };

        obj = load_from_archive_start(ar, registry, on_create);
    }

    return obj;
}
auto load_from_prefab_bin(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle
{
    entt::handle obj;

    const auto& prefab = pfb.get();
    auto buffer = prefab->buffer.get_stream_buf();
    std::istream stream(&buffer);
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        ser20::iarchive_binary_t ar(stream);

        auto on_create = [&pfb](entt::handle obj)
        {
            if(obj)
            {
                auto& pfb_comp = obj.get_or_emplace<prefab_component>();
                pfb_comp.source = pfb;
            }
        };

        obj = load_from_archive_start(ar, registry, on_create);
    }

    return obj;
}

void clone_entity_from_stream(entt::const_handle src_obj, entt::handle& dst_obj)
{
    APPLOG_INFO_PERF(std::chrono::microseconds);

    std::stringstream ss;
    save_to_stream(ss, src_obj);

    ss.seekp(0);
    ss.seekg(0);

    load_from_stream(ss, dst_obj);
}

void save_to_stream(std::ostream& stream, const scene& scn)
{
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_oarchive_associative(stream);
        save_to_archive(ar, *scn.registry);
    }
}
void save_to_file(const std::string& absolute_path, const scene& scn)
{
    APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ofstream stream(absolute_path);
    save_to_stream(stream, scn);
}
void save_to_stream_bin(std::ostream& stream, const scene& scn)
{
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        ser20::oarchive_binary_t ar(stream);
        save_to_archive(ar, *scn.registry);
    }
}
void save_to_file_bin(const std::string& absolute_path, const scene& scn)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    save_to_stream_bin(stream, scn);
}

void load_from_view(std::string_view view, scene& scn)
{
    if(!view.empty())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(view.data(), view.size());
        load_from_archive(ar, *scn.registry);
    }
}

void load_from_stream(std::istream& stream, scene& scn)
{
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        auto ar = ser20::create_iarchive_associative(stream);
        load_from_archive(ar, *scn.registry);
    }
}
void load_from_file(const std::string& absolute_path, scene& scn)
{
    APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ifstream stream(absolute_path);
    load_from_stream(stream, scn);
}
void load_from_stream_bin(std::istream& stream, scene& scn)
{
    if(stream.good())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);

        ser20::iarchive_binary_t ar(stream);
        load_from_archive(ar, *scn.registry);
    }
}
void load_from_file_bin(const std::string& absolute_path, scene& scn)
{
    APPLOG_INFO_PERF(std::chrono::microseconds);

    std::ifstream stream(absolute_path, std::ios::binary);
    load_from_stream_bin(stream, scn);
}

auto load_from_prefab(const asset_handle<scene_prefab>& pfb, scene& scn) -> bool
{
    const auto& prefab = pfb.get();
    const auto& buffer = prefab->buffer.data;

    if(!buffer.empty())
    {
        APPLOG_INFO_PERF(std::chrono::microseconds);
        auto ar = ser20::create_iarchive_associative(buffer.data(), buffer.size());
        load_from_archive(ar, *scn.registry);
    }

    return true;
}
auto load_from_prefab_bin(const asset_handle<scene_prefab>& pfb, scene& scn) -> bool
{
    const auto& prefab = pfb.get();
    auto buffer = prefab->buffer.get_stream_buf();

    APPLOG_INFO_PERF(std::chrono::microseconds);
    std::istream stream(&buffer);
    if(!stream.good())
    {
        return false;
    }

    load_from_stream_bin(stream, scn);

    return true;
}

void clone_scene_from_stream(const scene& src_scene, scene& dst_scene)
{
    dst_scene.unload();

    auto& src = src_scene.registry;
    auto& dst = dst_scene.registry;

    APPLOG_INFO_PERF(std::chrono::microseconds);

    src->view<transform_component, root_component>().each(
        [&](auto e, auto&& comp1, auto&& comp2)
        {
            std::stringstream ss;
            save_to_stream(ss, src_scene.create_entity(e));

            ss.seekp(0);
            ss.seekg(0);
            auto e_clone = dst_scene.registry->create();
            auto e_clone_obj = dst_scene.create_entity(e_clone);

            load_from_stream(ss, e_clone_obj);
        });
}
} // namespace ace
