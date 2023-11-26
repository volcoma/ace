#pragma once

#include <string>
#include <vector>

namespace string_utils
{

using string_tokens_t = std::vector<std::string>;

auto ltrim(const std::string& str) -> std::string;

auto rtrim(const std::string& str) -> std::string;

auto trim(const std::string& str) -> std::string;

auto replace(const std::string& subject, const std::string& search, const std::string& replace) -> std::string;

auto to_upper(const std::string& str) -> std::string;

auto to_lower(const std::string& str) -> std::string;

// Returns the first substring between "from - to" without trailing spaces
auto extract_substring(const std::string& str, const std::string& from, const std::string& to) -> std::string;

namespace alterable
{
void ltrim(std::string& str);

void rtrim(std::string& str);

void trim(std::string& str);

void replace(std::string& subject, const std::string& search, const std::string& replace);

void to_upper(std::string& str);

void to_lower(std::string& str);
} // namespace alterable

} // namespace string_utils
