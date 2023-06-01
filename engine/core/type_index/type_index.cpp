#include "type_index.h"

namespace rtti
{
auto type_index::get_external() -> size_t
{
    static size_t index = 0;
    return ++index;
}

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
#if HPP_TYPE_INDEX_RTTI_ENABLED
    return std::type_index(*info_).hash_code();
#else
    return info_;
#endif
}

} // namespace rtti
