#include "type_index.h"

namespace rtti
{

type_index::type_index(type_index::construct_t info) noexcept : info_{info}
{
}

auto type_index::operator==(const type_index& o) const noexcept -> bool
{
    return hash_code() == o.hash_code();
}

auto type_index::operator!=(const type_index& o) const noexcept -> bool
{
    return hash_code() != o.hash_code();
}

auto type_index::operator<(const type_index& o) const noexcept -> bool
{
    return hash_code() < o.hash_code();
}

auto type_index::operator>(const type_index& o) const noexcept -> bool
{
    return hash_code() > o.hash_code();
}

auto type_index::hash_code() const noexcept -> std::size_t
{
    return info_.hash_code;
}

auto type_index::name() const noexcept -> hpp::string_view
{
    return info_.name;
}

} // namespace rtti
