#include "utils.h"
#include <algorithm>

namespace string_utils
{

auto ltrim(const std::string& str) -> std::string
{
    auto str_modified = str;
    alterable::ltrim(str_modified);
    return str_modified;
}

auto rtrim(const std::string& str) -> std::string
{
    auto str_modified = str;
    alterable::rtrim(str_modified);
    return str_modified;
}

auto trim(const std::string& str) -> std::string
{
    auto str_modified = str;
    alterable::trim(str_modified);
    return str_modified;
}

auto replace(const std::string& str, const std::string& search, const std::string& replace) -> std::string
{
    auto str_modified = str;
    alterable::replace(str_modified, search, replace);
    return str_modified;
}

auto to_upper(const std::string& str) -> std::string
{
    auto str_modified = str;
    alterable::to_upper(str_modified);
    return str_modified;
}

auto to_lower(const std::string& str) -> std::string
{
    auto str_modified = str;
    alterable::to_lower(str_modified);
    return str_modified;
}

auto extract_substring(const std::string& str, const std::string& from, const std::string& to) -> std::string
{
    auto it_from = str.find(from);
    if(it_from != std::string::npos)
    {
        auto result = str.substr(it_from + from.size(), std::string::npos);
        auto it_to = result.find_first_of(to);
        if(it_to != std::string::npos)
        {
            result = result.substr(0, it_to);
            alterable::trim(result);
            return result;
        }
    }

    return {};
}

namespace alterable
{
void ltrim(std::string& str)
{
    str.erase(str.begin(),
              std::find_if(str.begin(),
                           str.end(),
                           [](unsigned char ch)
                           {
                               return !std::isspace(ch);
                           }));
}

void rtrim(std::string& str)
{
    str.erase(std::find_if(str.rbegin(),
                           str.rend(),
                           [](unsigned char ch)
                           {
                               return !std::isspace(ch);
                           })
                  .base(),
              str.end());
}

void trim(std::string& str)
{
    alterable::ltrim(str);
    alterable::rtrim(str);
}

void replace(std::string& str, const std::string& search, const std::string& replace)
{
    if(search.empty())
    {
        return;
    }

    size_t pos = 0;
    while((pos = str.find(search, pos)) != std::string::npos)
    {
        str.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

void to_upper(std::string& str)
{
    std::transform(std::begin(str),
                   std::end(str),
                   std::begin(str),
                   [](unsigned char c)
                   {
                       return std::toupper(c);
                   });
}

void to_lower(std::string& str)
{
    std::transform(std::begin(str),
                   std::end(str),
                   std::begin(str),
                   [](unsigned char c)
                   {
                       return std::tolower(c);
                   });
}
} // namespace alterable

// namespace alterable

} // namespace string_utils
