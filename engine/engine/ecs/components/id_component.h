#pragma once

#include <string>
#include <uuid/uuid.h>

namespace ace
{

struct id_component
{
    std::string name{};
    hpp::uuid id = generate_uuid();
};

} // namespace ace
