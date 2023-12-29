#pragma once

#include <memory>
#include <iosfwd>

namespace ace
{

struct prefab
{
    std::shared_ptr<std::istream> data;
};

} // namespace ace
