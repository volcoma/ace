#include "asset_database.hpp"

#include <engine/meta/core/common/basetypes.hpp>

#include <fstream>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>
#include <serialization/types/map.hpp>

namespace ace
{
SAVE(asset_database::meta)
{
    try_save(ar, ser20::make_nvp("location", obj.location));
}
SAVE_INSTANTIATE(asset_database::meta, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(asset_database::meta, ser20::oarchive_binary_t);

LOAD(asset_database::meta)
{
    try_load(ar, ser20::make_nvp("location", obj.location));
}
LOAD_INSTANTIATE(asset_database::meta, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(asset_database::meta, ser20::iarchive_binary_t);

SAVE(asset_database)
{
    const auto& db = obj.get_database();
    try_save(ar, ser20::make_nvp("database", db));
}
SAVE_INSTANTIATE(asset_database, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(asset_database, ser20::oarchive_binary_t);

LOAD(asset_database)
{
    asset_database::database_t database;

    try_load(ar, ser20::make_nvp("database", database));

    obj.set_database(database);
}
LOAD_INSTANTIATE(asset_database, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(asset_database, ser20::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const asset_database& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        ser20::oarchive_associative_t ar(stream);
        try_save(ar, ser20::make_nvp("db", obj));
    }
}
void save_to_file_bin(const std::string& absolute_path, const asset_database& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("db", obj));
    }
}
auto load_from_file(const std::string& absolute_path, asset_database& obj) -> bool
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        ser20::iarchive_associative_t ar(stream);
        return try_load(ar, ser20::make_nvp("db", obj));
    }
    return false;
}
auto load_from_file_bin(const std::string& absolute_path, asset_database& obj) -> bool
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::iarchive_binary_t ar(stream);
        return try_load(ar, ser20::make_nvp("db", obj));
    }
    return false;
}



SAVE(asset_meta)
{
    try_save(ar, ser20::make_nvp("type", obj.type));
    try_save(ar, ser20::make_nvp("uid", obj.uid));

}
SAVE_INSTANTIATE(asset_meta, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(asset_meta, ser20::oarchive_binary_t);

LOAD(asset_meta)
{
    try_load(ar, ser20::make_nvp("type", obj.type));
    try_load(ar, ser20::make_nvp("uid", obj.uid));
}
LOAD_INSTANTIATE(asset_meta, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(asset_meta, ser20::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const asset_meta& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        try
        {
            ser20::oarchive_associative_t ar(stream);
            try_save(ar, ser20::make_nvp("meta", obj));
        }
        catch(const ser20::Exception& e)
        {
            serialization::log_warning(e.what());
        }
    }
}
void save_to_file_bin(const std::string& absolute_path, const asset_meta& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        try
        {
            ser20::oarchive_binary_t ar(stream);
            try_save(ar, ser20::make_nvp("meta", obj));
        }
        catch(const ser20::Exception& e)
        {
            serialization::log_warning(e.what());
        }
    }
}
auto load_from_file(const std::string& absolute_path, asset_meta& obj) -> bool
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        try
        {
            ser20::iarchive_associative_t ar(stream);
            return try_load(ar, ser20::make_nvp("meta", obj));
        }
        catch(const ser20::Exception& e)
        {
            serialization::log_warning(e.what());
        }
    }
    return false;
}
auto load_from_file_bin(const std::string& absolute_path, asset_meta& obj) -> bool
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        try
        {
            ser20::iarchive_binary_t ar(stream);
            return try_load(ar, ser20::make_nvp("meta", obj));
        }
        catch(const ser20::Exception& e)
        {
            serialization::log_warning(e.what());
        }
    }
    return false;
}
} // namespace ace
