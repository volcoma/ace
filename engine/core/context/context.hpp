#ifndef HPP_CONTEXT
#define HPP_CONTEXT

#include <type_index/type_index.h>
#include <hpp/type_name.hpp>

#include <memory>
#include <unordered_map>
#include <iostream>

namespace rtti
{

struct context
{
    template<typename T, typename... Args>
    auto add(Args&&... args) -> T&
    {
        auto index = rtti::type_id<T>().hash_code();
//        std::cout << "context::" << __func__ << " < " << hpp::type_name_str<T>() << " >() -> " << index << std::endl;

        auto obj = std::make_shared<T>(std::forward<Args>(args)...);
        objects_[index] = obj;
        return *obj;
    }

    template<typename T>
    auto get() -> T&
    {
        const auto index = rtti::type_id<T>().hash_code();
        return *reinterpret_cast<T*>(objects_.at(index).get());
    }

    template<typename T>
    auto get() const -> const T&
    {
        const auto index = rtti::type_id<T>().hash_code();
        return *reinterpret_cast<const T*>(objects_.at(index).get());
    }

    template<typename T>
    void remove()
    {
        const auto index = rtti::type_id<T>().hash_code();
//        std::cout << "context::" << __func__ << " < " << hpp::type_name_str<T>() << " >() -> " << index << std::endl;
        objects_.erase(index);
    }

    std::unordered_map<std::size_t, std::shared_ptr<void>> objects_;
};

} // namespace rtti
#endif
