#include "reflection.h"

namespace rttr
{
auto get_pretty_name(type t) -> std::string
{
    std::string name = t.get_name().to_string();
    auto meta_id = t.get_metadata("pretty_name");
    if(meta_id)
    {
        name = meta_id.to_string();
    }
    return name;
}

auto get_pretty_name(const rttr::property& prop) -> std::string
{
    std::string name = prop.get_name().to_string();
    auto meta_id = prop.get_metadata("pretty_name");
    if(meta_id)
    {
        name = meta_id.to_string();
    }
    return name;
}
} // namespace rttr

auto register_type_helper(const char* name) -> int
{
      return 0;
}
