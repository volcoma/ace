#pragma once

#include <context/context.hpp>
#include <reflection/reflection.h>
#include <reflection/registration.h>

#include <editor/imgui/integration/imgui.h>

namespace ace
{
class property_layout
{
public:
    property_layout();
    property_layout(const rttr::property& prop, bool columns = true);
    property_layout(const std::string& name, bool columns = true);
    property_layout(const std::string& name, const std::string& tooltip, bool columns = true);

    ~property_layout();

    void set_data(const rttr::property& prop, bool columns = true);
    void set_data(const std::string& name, const std::string& tooltip, bool columns = true);

    void push_layout();
    auto push_tree_layout(ImGuiTreeNodeFlags flags = 0) -> bool;
    void pop_layout();

    static auto get_current() -> property_layout*;

private:
    bool pushed_{};
    std::string name_;
    std::string tooltip_;
    bool columns_{};
    bool open_{};
};

struct var_info
{
    bool read_only{};
    bool is_property{};
};

struct inspect_result
{
    bool changed{};
    bool edit_finished{};

    auto operator|=(const inspect_result& rhs) -> inspect_result&
    {
        changed |= rhs.changed;
        edit_finished |= rhs.edit_finished;

        return *this;
    }

    auto operator|(const inspect_result& rhs) const -> inspect_result
    {
        inspect_result result{};
        result.changed |= rhs.changed;
        result.edit_finished |= rhs.edit_finished;

        return result;
    }
};

struct inspector
{
    REFLECTABLEV(inspector)

    using meta_getter = std::function<rttr::variant(const rttr::variant&)>;

    virtual ~inspector() = default;

    virtual void before_inspect(const rttr::property& prop);
    virtual void after_inspect(const rttr::property& prop);
    virtual auto inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata)
        -> inspect_result = 0;

    std::unique_ptr<property_layout> layout_{};
};

REFLECT_INLINE(inspector)
{
    rttr::registration::class_<inspector>("inspector");
}
#define REFLECT_INSPECTOR_INLINE(inspector_type, inspected_type)                                                       \
    REFLECT_INLINE(inspector_type)                                                                                     \
    {                                                                                                                  \
        rttr::registration::class_<inspector_type>(#inspector_type)(                                                   \
            rttr::metadata("inspected_type", rttr::type::get<inspected_type>()))                                       \
            .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);                                                   \
    }

#define REFLECT_INSPECTOR(inspector_type, inspected_type)                                                              \
    REFLECT(inspector_type)                                                                                            \
    {                                                                                                                  \
        rttr::registration::class_<inspector_type>(#inspector_type)(                                                   \
            rttr::metadata("inspected_type", rttr::type::get<inspected_type>()))                                       \
            .constructor<>()(rttr::policy::ctor::as_std_shared_ptr);                                                   \
    }

} // namespace ace
