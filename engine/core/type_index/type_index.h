#ifndef HPP_TYPE_INDEX
#define HPP_TYPE_INDEX

#include <cstddef>
#include <future>
#include <type_traits>

#include <hpp/type_name.hpp>

///////////////////////////////////////////////////////
#ifndef HPP_TYPE_INDEX_USE_RTTI
#define HPP_TYPE_INDEX_USE_RTTI 0
#endif
///////////////////////////////////////////////////////

#ifdef _MSC_VER
#ifndef HPP_TYPE_INDEX_RTTI
#define HPP_TYPE_INDEX_RTTI _CPPRTTI
#endif // !__cpp_rtti
#endif

#define HPP_TYPE_INDEX_RTTI_ENABLED HPP_TYPE_INDEX_USE_RTTI&& HPP_TYPE_INDEX_RTTI
#if HPP_TYPE_INDEX_RTTI_ENABLED
#include <typeindex>
#endif

namespace rtti
{

class type_index
{
#if HPP_TYPE_INDEX_RTTI_ENABLED
    using construct_t = const std::type_info*;
#else
    using construct_t = size_t;
#endif
    construct_t info_ = {};
    explicit type_index(construct_t info) noexcept;

    template<typename T>
    static auto get() -> const type_index&;

    static auto get_external() -> size_t;

    template<typename T>
    friend auto type_id() -> const type_index&;

public:
    auto operator==(const type_index& o) const noexcept -> bool;
    auto operator!=(const type_index& o) const noexcept -> bool;
    auto operator<(const type_index& o) const noexcept -> bool;
    auto operator>(const type_index& o) const noexcept -> bool;

    auto hash_code() const noexcept -> std::size_t;
};

template<typename T>
auto type_index::get() -> const type_index&
{
#if HPP_TYPE_INDEX_RTTI_ENABLED
    static const type_index id(&typeid(type_t));
    return id;
#else
    static std::hash<std::string> hasher;
    static const type_index id(hasher(hpp::type_name_str<T>()));
    return id;
#endif
}

template<typename T>
auto type_id() -> const type_index&
{
    using type_t = typename std::remove_cv<T>::type;
    return type_index::get<type_t>();
}

} // namespace rtti

#endif
