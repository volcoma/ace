#pragma once

#include <memory>
#include <iosfwd>
#include <vector>
#include <istream>

namespace ace
{

struct prefab
{
    class membuf : public std::streambuf
    {
    public:
        membuf(const uint8_t* begin, size_t size) {
            auto cbegin = reinterpret_cast<char*>(const_cast<uint8_t*>(begin));
            this->setg(cbegin, cbegin, cbegin + size);
        }
    };
    auto get_stream_buf() const -> membuf
    {
        membuf result(data.data(), data.size());
        return result;
    }

    std::vector<uint8_t> data;
};

struct scene_prefab : prefab
{

};

} // namespace ace
