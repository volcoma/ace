#ifndef REFLECTION_H
#define REFLECTION_H

#include <reflection/reflection_export.h>
#include <rttr/array_range.h>
#include <rttr/constructor.h>
#include <rttr/destructor.h>
#include <rttr/enumeration.h>
#include <rttr/method.h>
#include <rttr/property.h>
#include <rttr/registration.h>
#include <rttr/rttr_cast.h>
#include <rttr/rttr_enable.h>
#include <rttr/type.h>

#define CAT_IMPL_(a, b) a##b
#define CAT_(a, b)      CAT_IMPL_(a, b)
#ifdef __COUNTER__
#define ANONYMOUS_VARIABLE(str) CAT_(str, CAT_(__COUNTER__, CAT_(__LINE__, __COUNTER__)))
#else
#define ANONYMOUS_VARIABLE(str) CAT_(str, __LINE__)
#endif

// REFLECTION_EXPORT auto register_type_helper(const char*) -> int;

// #define REFLECT(cls)                                                                                                   \
//     static void rttr_auto_register_reflection_function_##cls();                                                        \
//     static const int ANONYMOUS_VARIABLE(auto_register__) = []()                                                        \
//     {                                                                                                                  \
//         rttr_auto_register_reflection_function_##cls();                                                                \
//         return register_type_helper(#cls);                                                                             \
//     }();                                                                                                               \
//     static void rttr_auto_register_reflection_function_##cls()

// #define REFLECT_INLINE(cls)                                                                                            \
//     void rttr_auto_register_reflection_function_##cls();                                                               \
//     inline int ANONYMOUS_VARIABLE(auto_register__) = []()                                                        \
//     {                                                                                                                  \
//         rttr_auto_register_reflection_function_##cls();                                                                \
//         return register_type_helper(#cls);                                                                             \
//     }();                                                                                                               \
//     inline void rttr_auto_register_reflection_function_##cls()


namespace refl_detail
{
template<typename T>
inline int get_reg(void (*f)())
{
    static const int s = [&f]()
    {
        f();
        return 0;
    }();
    return s;
}
} // namespace refl_detail

#define REFLECT_INLINE(cls)                                                                                            \
    template<typename T>                                                                                               \
    extern void rttr_auto_register_reflection_function_t();                                                            \
    template<>                                                                                                         \
    void rttr_auto_register_reflection_function_t<cls>();                                                              \
    static const int ANONYMOUS_VARIABLE(auto_register__) =                                                             \
        refl_detail::get_reg<cls>(&rttr_auto_register_reflection_function_t<cls>);                                     \
    template<>                                                                                                         \
    inline void rttr_auto_register_reflection_function_t<cls>()

#define REFLECT_EXTERN(cls)                                                                                            \
    template<typename T>                                                                                               \
    extern void rttr_auto_register_reflection_function_t();                                                            \
    template<>                                                                                                         \
    void rttr_auto_register_reflection_function_t<cls>();                                                              \
    static const int ANONYMOUS_VARIABLE(auto_register__) =                                                             \
        refl_detail::get_reg<cls>(&rttr_auto_register_reflection_function_t<cls>)

#define REFLECT(cls)                                                                                                   \
    template<>                                                                                                         \
    void rttr_auto_register_reflection_function_t<cls>()
namespace rttr
{
REFLECTION_EXPORT auto get_pretty_name(type t) -> std::string;
REFLECTION_EXPORT auto get_pretty_name(const rttr::property& prop) -> std::string;
} // namespace rttr
#endif // RTTR_REFLECTION_H_
