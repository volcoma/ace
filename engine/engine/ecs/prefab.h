#pragma once

#include <memory>
#include <iosfwd>
#include <vector>
#include <istream>
#include <filesystem/filesystem.h>

namespace ace
{

struct prefab
{    
    fs::stream_buffer<std::vector<uint8_t>> buffer{};
};

struct scene_prefab : prefab
{

};

} // namespace ace
