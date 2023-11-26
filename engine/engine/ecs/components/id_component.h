#pragma once

#include <string>
#include <uuid/uuid.h>

namespace ace
{

struct id_component
{
    hpp::uuid id = generate_uuid();
};

struct tag_component
{
    std::string tag{};
};

} // namespace ace
