#pragma once

#include <context/context.hpp>
#include <reflection/reflection.h>
#include <reflection/registration.h>

#include <editor/imgui/integration/imgui.h>

namespace ace
{
struct property_layout
{
    property_layout(const rttr::property& prop, bool columns = true);

    property_layout(const std::string& name, bool columns = true);

    property_layout(const std::string& name, const std::string& tooltip, bool columns = true);

    ~property_layout();

    void push_layout();
    void pop_layout();

    std::string name_;
    std::string tooltip_;
    bool columns_{};
};


struct var_info
{
    bool read_only{};
    bool is_property{};
};

struct inspector
{
    REFLECTABLEV(inspector)

    using meta_getter = std::function<rttr::variant(const rttr::variant&)>;

    virtual ~inspector() = default;

    virtual void before_inspect(const rttr::property& prop);
    virtual void after_inspect(const rttr::property& prop);
    virtual bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata) = 0;

    std::unique_ptr<property_layout> layout_{};
};

REFLECT_INLINE(inspector)
{
    rttr::registration::class_<inspector>("inspector");
}
#define INSPECTOR_REFLECT(inspector_type, inspected_type)                                                              \
    REFLECT_INLINE(inspector_type)                                                                                     \
    {                                                                                                                  \
        rttr::registration::class_<inspector_type>(#inspector_type)(                                                   \
            rttr::metadata("inspected_type", rttr::type::get<inspected_type>()))                                       \
            .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);                                                   \
    }

#define DECLARE_INSPECTOR(inspector_type, inspected_type)                                                              \
    struct inspector_type : public inspector                                                                           \
    {                                                                                                                  \
        REFLECTABLEV(inspector_type, inspector)                                                                        \
        bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);                             \
    };                                                                                                                 \
    INSPECTOR_REFLECT(inspector_type, inspected_type)
}
