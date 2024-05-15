#ifndef REFLECTION_REGISTRATION_H
#define REFLECTION_REGISTRATION_H

#include <rttr/registration_friend.h>
#include <rttr/rttr_enable.h>

#define RTTR_REGISTRATION_FRIEND_NON_INTRUSIVE(cls)                                                                    \
    template<typename T>                                                                                               \
    friend void rttr_auto_register_reflection_function_t();                                                            \
    RTTR_REGISTRATION_FRIEND

#define REFLECTABLE(cls)                                                                                               \
    RTTR_REGISTRATION_FRIEND_NON_INTRUSIVE(cls)                                                                        \
    RTTR_ENABLE()                                                                                                      \
public:

#define REFLECTABLEV(cls, ...)                                                                                         \
    RTTR_REGISTRATION_FRIEND_NON_INTRUSIVE(cls)                                                                        \
    RTTR_ENABLE(__VA_ARGS__)                                                                                           \
public:
#endif // REFLECTION_REGISTRATION_H
