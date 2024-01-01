#include "entity.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include "components/camera_component.hpp"
#include "components/id_component.hpp"
#include "components/light_component.hpp"
#include "components/model_component.hpp"
#include "components/prefab_component.hpp"
#include "components/reflection_probe_component.hpp"
#include "components/test_component.hpp"
#include "components/transform_component.hpp"

#include <hpp/utility.hpp>

namespace ace
{

struct entity_loader
{
    entt::registry* reg{};
    std::map<entt::entity, entt::handle> mapping;
};

struct entity_components
{
    entt::handle entity;
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

namespace cereal
{

SAVE(entt::handle)
{
    ar(cereal::make_nvp("id", obj.entity()));
}
SAVE_INSTANTIATE(entt::handle, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(entt::handle, cereal::oarchive_binary_t);

LOAD(entt::handle)
{
    entt::handle::entity_type id{};
    ar(cereal::make_nvp("id", id));

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

LOAD_INSTANTIATE(entt::handle, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(entt::handle, cereal::iarchive_binary_t);

SAVE(entity_components)
{
    auto components = obj.entity.try_get<tag_component,
                                         transform_component,
                                         test_component,
                                         model_component,
                                         camera_component,
                                         light_component,
                                         reflection_probe_component>();

    hpp::for_each(components,
                  [&](auto& component)
                  {
                      using ctype = std::decay_t<decltype(*component)>;

                      auto component_type = rttr::type::get<ctype>();
                      std::string name = component_type.get_name().data();
                      auto meta_id = component_type.get_metadata("pretty_name");
                      if(meta_id)
                      {
                          name = meta_id.to_string();
                      }

                      auto has_name = "Has" + name;
                      ar(cereal::make_nvp(has_name, component != nullptr));

                      if(component)
                      {
                          ar(cereal::make_nvp(name, *component));
                      }
                  });
}
SAVE_INSTANTIATE(entity_components, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(entity_components, cereal::oarchive_binary_t);

LOAD(entity_components)
{
    hpp::for_each_type<tag_component,
                       transform_component,
                       test_component,
                       model_component,
                       camera_component,
                       light_component,
                       reflection_probe_component>(
        [&](auto tag)
        {
            using ctype = std::decay_t<decltype(tag)>::type;

            auto component_type = rttr::type::get<ctype>();
            std::string name = component_type.get_name().data();
            auto meta_id = component_type.get_metadata("pretty_name");
            if(meta_id)
            {
                name = meta_id.to_string();
            }

            auto has_name = "Has" + name;
            bool has_component = false;
            ar(cereal::make_nvp(has_name, has_component));

            if(has_component)
            {
                auto& component = obj.entity.emplace_or_replace<ctype>();
                ar(cereal::make_nvp(name, component));
            }
        });
}
LOAD_INSTANTIATE(entity_components, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(entity_components, cereal::iarchive_binary_t);

} // namespace cereal

namespace ace
{
namespace
{
template<typename Archive>
void save_to_archive(Archive& ar, entt::handle obj)
{
    auto& trans_comp = obj.get<transform_component>();

    const auto& children = trans_comp.get_children();

    SAVE_FUNCTION_NAME(ar, obj);
    ar(cereal::make_nvp("components", entity_components{obj}));

    for(const auto& child : children)
    {
        save_to_archive(ar, child);
    }
}

template<typename Archive>
auto load_from_archive(Archive& ar, entt::registry& registry, const std::function<void(entt::handle)>& on_create)
    -> entt::handle
{
    entt::handle obj;
    LOAD_FUNCTION_NAME(ar, obj);


    auto& rel = obj.emplace<hierarchy_component>();
    entity_components components{obj};
    ar(cereal::make_nvp("components", components));


    set_parent_params params;
    params.local_transform_stays = true;
    params.global_transform_stays = false;

    auto& trans_comp = obj.get<transform_component>();
    trans_comp.set_parent(rel.parent, params);
    for(auto& child : rel.children)
    {
        child = load_from_archive(ar, registry, on_create);
    }

    obj.remove<hierarchy_component>();

    if(on_create)
    {
        on_create(obj);
    }

    return obj;
}

template<typename Archive>
auto load_from_archive_start(Archive& ar,
                             entt::registry& registry,
                             const std::function<void(entt::handle)>& on_create = {}) -> entt::handle
{
    entity_loader loader;
    loader.reg = &registry;

    set_loader(loader);

    auto obj = load_from_archive(ar, registry, on_create);

    reset_loader();

    return obj;
}
} // namespace

void save_to_file(const std::string& absolute_path, entt::handle obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        cereal::oarchive_associative_t ar(stream);

        save_to_archive(ar, obj);
    }
}
void save_to_file_bin(const std::string& absolute_path, entt::handle obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        cereal::oarchive_binary_t ar(stream);

        save_to_archive(ar, obj);
    }
}

void load_from_file(const std::string& absolute_path, entt::handle& obj)
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        cereal::iarchive_associative_t ar(stream);
        obj = load_from_archive_start(ar, *obj.registry());
    }
}
void load_from_file_bin(const std::string& absolute_path, entt::handle& obj)
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        cereal::iarchive_binary_t ar(stream);
        obj = load_from_archive_start(ar, *obj.registry());
    }
}

auto load_from_prefab(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle
{
    entt::handle obj;
    auto& stream = *pfb.get().data;
    if(stream.good())
    {
        cereal::iarchive_associative_t ar(stream);

        auto on_create = [&pfb](entt::handle obj)
        {
            if(obj)
            {
                auto& pfb_comp = obj.get_or_emplace<prefab_component>();
                pfb_comp.source = pfb;
            }
        };

        obj = load_from_archive_start(ar, registry, on_create);

        stream.seekg(0);
    }

    return obj;
}
auto load_from_prefab_bin(const asset_handle<prefab>& pfb, entt::registry& registry) -> entt::handle
{
    entt::handle obj;

    auto& stream = *pfb.get().data;

    if(stream.good())
    {
        cereal::iarchive_binary_t ar(stream);

        auto on_create = [&pfb](entt::handle obj)
        {
            if(obj)
            {
                auto& pfb_comp = obj.get_or_emplace<prefab_component>();
                pfb_comp.source = pfb;
            }
        };

        obj = load_from_archive_start(ar, registry, on_create);

        stream.seekg(0);
    }

    return obj;
}
} // namespace ace
