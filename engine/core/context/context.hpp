#ifndef HPP_CONTEXT
#define HPP_CONTEXT

#include <hpp/type_index.hpp>

#include <iostream>
#include <map>
#include <memory>

namespace rtti
{

struct context
{
    template<typename T, typename D = T, typename... Args>
    auto add(Args&&... args) -> T&
    {
        const auto id = hpp::type_id<T>();
        //        std::cout << "context::" << __func__ << " < " << hpp::type_name_str<T>() << " >() -> " << index <<
        //        std::endl;

        std::shared_ptr<T> obj = std::make_shared<D>(std::forward<Args>(args)...);
        objects_[id] = obj;
        return *obj;
    }

    template<typename T>
    auto get() -> T&
    {
        const auto id = hpp::type_id<T>();
        return *reinterpret_cast<T*>(objects_.at(id).get());
    }

    template<typename T>
    auto get() const -> const T&
    {
        const auto id = hpp::type_id<T>();
        return *reinterpret_cast<const T*>(objects_.at(id).get());
    }

    template<typename T>
    void remove()
    {
        const auto id = hpp::type_id<T>();
        //        std::cout << "context::" << __func__ << " < " << hpp::type_name_str<T>() << " >() -> " << index <<
        //        std::endl;
        objects_.erase(id);
    }

    auto empty() const -> bool
    {
        return objects_.empty();
    }

    void print_types() const
    {
        for(const auto& kvp : objects_)
        {
            std::cout << " < " << kvp.first.name() << " >() -> " << kvp.first.hash_code() << std::endl;
        }
    }

private:
    std::map<hpp::type_index, std::shared_ptr<void>> objects_;
};

} // namespace rtti
#endif
