#pragma once
#include "ser20/access.hpp"
#include "ser20/ser20.hpp"
#include "ser20/types/polymorphic.hpp"
#include "ser20/types/vector.hpp"
#include <hpp/source_location.hpp>

#include <functional>
#include <string>

#define SERIALIZE_FUNCTION_NAME                    SER20_SERIALIZE_FUNCTION_NAME
#define SAVE_FUNCTION_NAME                         SER20_SAVE_FUNCTION_NAME
#define LOAD_FUNCTION_NAME                         SER20_LOAD_FUNCTION_NAME
#define SAVE_MINIMAL_FUNCTION_NAME                 SER20_SAVE_MINIMAL_FUNCTION_NAME
#define LOAD_MINIMAL_FUNCTION_NAME                 SER20_LOAD_MINIMAL_FUNCTION_NAME
#define SERIALIZE_REGISTER_TYPE_WITH_NAME(T, Name) SER20_REGISTER_TYPE_WITH_NAME(T, Name)
namespace serialization
{
using namespace ser20;

using log_callback_t = std::function<void(const std::string&, const hpp::source_location& loc)>;
void set_warning_logger(const log_callback_t& logger);
void log_warning(const std::string& log_msg, const hpp::source_location& loc = hpp::source_location::current());
} // namespace serialization

#define SERIALIZABLE(T)                                                                                                \
                                                                                                                       \
public:                                                                                                                \
    friend class serialization::access;                                                                                \
    template<typename Archive>                                                                                         \
    friend void SAVE_FUNCTION_NAME(Archive& ar, T const&);                                                             \
    template<typename Archive>                                                                                         \
    friend void LOAD_FUNCTION_NAME(Archive& ar, T&);

#define SERIALIZE_INLINE(cls)                                                                                          \
    template<typename Archive>                                                                                         \
    inline void SERIALIZE_FUNCTION_NAME(Archive& ar, cls& obj)

#define SAVE_INLINE(cls)                                                                                               \
    template<typename Archive>                                                                                         \
    inline void SAVE_FUNCTION_NAME(Archive& ar, cls const& obj)

#define LOAD_INLINE(cls)                                                                                               \
    template<typename Archive>                                                                                         \
    inline void LOAD_FUNCTION_NAME(Archive& ar, cls& obj)

#define SERIALIZE_EXTERN(cls)                                                                                          \
    template<typename Archive>                                                                                         \
    extern void SERIALIZE_FUNCTION_NAME(Archive& ar, cls& obj)

#define SAVE_EXTERN(cls)                                                                                               \
    template<typename Archive>                                                                                         \
    extern void SAVE_FUNCTION_NAME(Archive& ar, cls const& obj)

#define LOAD_EXTERN(cls)                                                                                               \
    template<typename Archive>                                                                                         \
    extern void LOAD_FUNCTION_NAME(Archive& ar, cls& obj)

#define SERIALIZE(cls)                                                                                                 \
    template<typename Archive>                                                                                         \
    void SERIALIZE_FUNCTION_NAME(Archive& ar, cls& obj)

#define SAVE(cls)                                                                                                      \
    template<typename Archive>                                                                                         \
    void SAVE_FUNCTION_NAME(Archive& ar, cls const& obj)

#define LOAD(cls)                                                                                                      \
    template<typename Archive>                                                                                         \
    void LOAD_FUNCTION_NAME(Archive& ar, cls& obj)

#define SERIALIZE_INSTANTIATE(cls, Archive) template void SERIALIZE_FUNCTION_NAME(Archive& archive, cls const& obj)

#define SAVE_INSTANTIATE(cls, Archive) template void SAVE_FUNCTION_NAME(Archive& archive, cls const& obj)

#define LOAD_INSTANTIATE(cls, Archive) template void LOAD_FUNCTION_NAME(Archive& archive, cls& obj)

template<typename Archive, typename T>
inline auto try_serialize(Archive& ar,
                          ser20::NameValuePair<T>&& t,
                          const hpp::source_location& loc = hpp::source_location::current()) -> bool
{
    try
    {
        ar(std::forward<ser20::NameValuePair<T>>(t));
    }
    catch(const ser20::Exception& e)
    {
        serialization::log_warning(e.what(), loc);
        return false;
    }
    return true;
}

template<typename Archive, typename T>
inline auto try_save(Archive& ar,
                     ser20::NameValuePair<T>&& t,
                     const hpp::source_location& loc = hpp::source_location::current()) -> bool
{
    return try_serialize(ar, std::forward<ser20::NameValuePair<T>>(t), loc);
}

template<typename Archive, typename T>
inline auto try_load(Archive& ar,
                     ser20::NameValuePair<T>&& t,
                     const hpp::source_location& loc = hpp::source_location::current()) -> bool
{
    return try_serialize(ar, std::forward<ser20::NameValuePair<T>>(t), loc);
}
