#pragma once

#include <hpp/uuid.hpp>

namespace ace
{

auto generate_uuid() -> hpp::uuid;
auto generate_uuid(const std::string& key) -> hpp::uuid;

}
