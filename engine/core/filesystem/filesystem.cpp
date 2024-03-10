#include "filesystem.h"
#include <algorithm>

namespace fs
{

namespace detail
{

bool is_parent_path(const path& parent, const path& child)
{
    return child.parent_path() == parent;
}

bool is_indirect_parent_path(const path& parent, const path& child)
{
    path rel = child.lexically_relative(parent);
    return !rel.empty() && rel.begin()->string() != "." && rel.begin()->string() != "..";
}

bool begins_with(const std::string& str, const std::string& value)
{
    // Validate requirements
    if(str.length() < value.length())
    {
        return false;
    }
    if(str.empty() || value.empty())
    {
        return false;
    }

    // Do the subsets match?
    auto s1 = str.substr(0, value.length());

    return s1.compare(value) == 0;
}
static std::string replace_seq(const std::string& str, const std::string& old_sequence, const std::string& new_sequence)
{
    std::string s = str;
    std::string::size_type location = 0;
    std::string::size_type old_length = old_sequence.length();
    std::string::size_type new_length = new_sequence.length();

    // Search for all replace std::string occurances.
    if(!s.empty())
    {
        while(std::string::npos != (location = s.find(old_sequence, location)))
        {
            s.replace(location, old_length, new_sequence);
            location += new_length;

            // Break out if we're done
            if(location >= s.length())
            {
                break;
            }

        } // Next

    } // End if not empty

    return s;
}

std::string to_lower(const std::string& str)
{
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    return s;
}

template<typename Container = std::string, typename CharT = char, typename Traits = std::char_traits<char>>
auto read_stream_into_container(std::basic_istream<CharT, Traits>& in, Container& container) -> bool
{
    static_assert(
        std::is_same<Container, std::basic_string<CharT, Traits, typename Container::allocator_type>>::value ||
            std::is_same<Container, std::vector<CharT, typename Container::allocator_type>>::value ||
            std::is_same<Container,
                         std::vector<std::make_unsigned_t<CharT>, typename Container::allocator_type>>::value ||
            std::is_same<Container, std::vector<std::make_signed_t<CharT>, typename Container::allocator_type>>::value,
        "only strings and vectors of ((un)signed) CharT allowed");

    auto const start_pos = in.tellg();
    if(std::streamsize(-1) == start_pos || !in.good())
    {
        return false;
    }

    if(!in.seekg(0, std::ios_base::end) || !in.good())
    {
        return false;
    }

    auto const end_pos = in.tellg();

    if(std::streamsize(-1) == end_pos || !in.good())
    {
        return false;
    }

    auto const char_count = end_pos - start_pos;

    if(!in.seekg(start_pos) || !in.good())
    {
        return false;
    }

    container.resize(static_cast<std::size_t>(char_count));

    if(!container.empty())
    {
        in.read(reinterpret_cast<CharT*>(&container[0]), char_count);
        container.resize(in.gcount());
    }

    return in.good() || in.eof();
}

} // namespace detail

bool is_case_insensitive()
{
    static bool is_insensitive = []()
    {
        auto timestamp = fs::file_time_type::clock::now();
        auto temp_path = fs::temp_directory_path();

        auto salt = std::to_string(timestamp.time_since_epoch().count()) + std::to_string(std::rand());
        std::string temp_name_lower = "_case_sensitivity_test_" + salt + ".txt";
        std::string temp_name_upper = "_CASE_SENSITIVITY_TEST_" + salt + ".txt";

        auto file_lower = temp_path / temp_name_lower;
        auto file_upper = temp_path / temp_name_upper;
        {
            std::ofstream os;
            os.open(file_lower);
        }

        fs::error_code ec;
        bool result = fs::equivalent(file_upper, file_lower, ec);
        fs::remove(file_lower, ec);

        return result;
    }();

    return is_insensitive;
}

byte_array_t read_stream(std::istream& stream)
{
    byte_array_t result{};
    detail::read_stream_into_container<byte_array_t>(stream, result);
    return result;
}

std::string read_stream_str(std::istream& stream)
{
    std::string result{};
    detail::read_stream_into_container<std::string>(stream, result);
    return result;
}

stream_buffer<byte_array_t> read_stream_buffer(std::istream& stream)
{
    stream_buffer<byte_array_t> result{};
    result.data = read_stream(stream);
    return result;
}

stream_buffer<std::string> read_stream_buffer_str(std::istream& stream)
{
    stream_buffer<std::string> result{};
    result.data = read_stream_str(stream);
    return result;
}

bool add_path_protocol(const std::string& protocol, const path& dir)
{
    // Protocol matching is case insensitive, convert to lower case
    auto protocol_lower = detail::to_lower(protocol);

    auto& protocols = get_path_protocols();
    // Add to the list
    protocols[protocol_lower] = fs::path(dir).make_preferred().string();

    // Success!
    return true;
}

protocols_t& get_path_protocols()
{
    static protocols_t protocols;
    return protocols;
}

path resolve_protocol(const path& _path)
{
    static const std::string separator = ":/";
    const auto string_path = _path.generic_string();
    auto pos = string_path.find(separator, 0);
    if(pos == std::string::npos)
    {
        return _path;
    }

    const auto root = string_path.substr(0, pos);

    fs::path relative_path = string_path.substr(pos + separator.size());
    // Matching path protocol in our list?
    const auto& protocols = get_path_protocols();

    auto it = protocols.find(root);

    if(it == std::end(protocols))
    {
        return _path;
    }

    auto result = path(it->second);
    if(!relative_path.empty())
    {
        result = result / relative_path.make_preferred();
    }
    return result;
}

bool has_known_protocol(const path& _path)
{
    static const std::string separator = ":/";

    const auto string_path = _path.generic_string();
    auto pos = string_path.find(separator, 0);
    if(pos == std::string::npos)
    {
        return false;
    }

    const auto root = string_path.substr(0, pos);

    const auto& protocols = get_path_protocols();

    // Matching path protocol in our list?
    return (protocols.find(root) != std::end(protocols));
}

path convert_to_protocol(const path& _path)
{
    const auto string_path = fs::path(_path).make_preferred().string();

    const auto& protocols = get_path_protocols();

    const protocols_t::value_type* best_protocol{};
    for(const auto& protocol_pair : protocols)
    {
        //        const auto& protocol = protocol_pair.first;
        const auto& resolved_protocol = protocol_pair.second;

        if(detail::begins_with(string_path, resolved_protocol))
        {
            if(best_protocol)
            {
                if(best_protocol->second.size() < resolved_protocol.size())
                {
                    best_protocol = &protocol_pair;
                }
            }
            else
            {
                best_protocol = &protocol_pair;
            }
        }
    }
    if(best_protocol)
    {
        const auto& protocol = best_protocol->first;
        const auto& resolved_protocol = best_protocol->second;

        return replace(string_path, resolved_protocol, protocol + ":/").generic_string();
    }
    return _path;
}

path replace(const path& _path, const path& _sequence, const path& _new_sequence)
{
    return path(detail::replace_seq(_path.string(), _sequence.string(), _new_sequence.string()));
}

std::vector<path> split_until(const path& _path, const path& _predicate)
{
    std::vector<path> result;

    auto f = _path;

    while(f.has_parent_path() && f.has_filename() && f != _predicate)
    {
        result.push_back(f);
        f = f.parent_path();
    }

    result.push_back(_predicate);
    std::reverse(std::begin(result), std::end(result));

    return result;
}

path reduce_trailing_extensions(const path& _path)
{
    fs::path reduced = _path;
    for(auto temp = reduced; temp.has_extension(); temp = reduced.stem())
    {
        reduced = temp;
    }

    fs::path result = _path;
    result.remove_filename();
    result /= reduced;
    return result;
}

bool is_any_parent_path(const path& parent, const path& child)
{
    return detail::is_parent_path(parent, child) || detail::is_indirect_parent_path(parent, child);
}
} // namespace fs
