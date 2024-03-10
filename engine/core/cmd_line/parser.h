#pragma once
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace cmd_line
{
namespace detail
{
auto get_internal_id() -> std::uint64_t;

template<typename T>
inline auto get_id() -> std::uint64_t
{
    static std::uint64_t id = get_internal_id();
    return id;
}
} // namespace detail

/// Class used to wrap integer types to specify desired numerical base for specific argument parsing
template<typename T, int num_base = 0>
class numeric_base
{
public:
    /// This constructor required for correct AgrumentCountChecker initialization
    numeric_base() : value(0), base(num_base)
    {
    }

    /// This constructor required for default value initialization
    /// \param val comes from default value
    numeric_base(T val) : value(val), base(num_base)
    {
    }

    operator T() const
    {
        return this->value;
    }
    operator T*()
    {
        return this->value;
    }

    T value;
    unsigned int base;
};

struct callback_args
{
    const std::vector<std::string>& arguments;
    std::ostream& output;
    std::ostream& error;
};

class parser
{
private:
    class cmd_base
    {
    public:
        explicit cmd_base(const std::string& name,
                          const std::string& alternative,
                          const std::string& description,
                          bool required,
                          bool dominant,
                          bool is_variadic);

        virtual ~cmd_base();

        std::string name;
        std::string command;
        std::string alternative;
        std::string description;
        bool required;
        bool handled;
        std::vector<std::string> arguments;
        bool const dominant;
        bool const variadic;

        virtual auto print_value() const -> std::string = 0;
        virtual auto parse(std::ostream& output, std::ostream& error) -> bool = 0;

        auto is(const std::string& given) const -> bool;
    };

    template<typename T>
    struct argument_count_checker
    {
        static constexpr bool variadic = false;
    };

    template<typename T>
    struct argument_count_checker<cmd_line::numeric_base<T>>
    {
        static constexpr bool variadic = false;
    };

    template<typename T>
    struct argument_count_checker<std::vector<T>>
    {
        static constexpr bool variadic = true;
    };

    template<typename T>
    class cmd_function final : public cmd_base
    {
    public:
        explicit cmd_function(const std::string& name,
                              const std::string& alternative,
                              const std::string& description,
                              bool required,
                              bool dominant)
            : cmd_base(name, alternative, description, required, dominant, argument_count_checker<T>::variadic)
        {
        }

        auto parse(std::ostream& output, std::ostream& error) -> bool override
        {
            try
            {
                callback_args args{arguments, output, error};
                value = callback(args);
                return true;
            }
            catch(...)
            {
                return false;
            }
        }

        auto print_value() const -> std::string override
        {
            return "";
        }

        std::function<T(callback_args&)> callback;
        T value;
    };

    template<typename T>
    class cmd_argument final : public cmd_base
    {
    public:
        explicit cmd_argument(const std::string& name,
                              const std::string& alternative,
                              const std::string& description,
                              bool required,
                              bool dominant)
            : cmd_base(name, alternative, description, required, dominant, argument_count_checker<T>::variadic)
        {
        }

        auto parse(std::ostream& /*output*/, std::ostream& /*error*/) -> bool override
        {
            try
            {
                value = parser::parse(arguments, value);
                return true;
            }
            catch(...)
            {
                return false;
            }
        }

        auto print_value() const -> std::string override
        {
            return stringify(value);
        }

        T value;
    };

    static auto parse(const std::vector<std::string>& elements, const int& /*unused*/, int numberBase = 0) -> int;

    static auto parse(const std::vector<std::string>& elements, const bool& defval) -> bool;

    static auto parse(const std::vector<std::string>& elements, const double& /*unused*/) -> double;

    static auto parse(const std::vector<std::string>& elements, const float& /*unused*/) -> float;

    static auto parse(const std::vector<std::string>& elements, const long double& /*unused*/) -> long double;

    static auto parse(const std::vector<std::string>& elements, const unsigned int& /*unused*/, int numberBase = 0)
        -> unsigned int;

    static auto parse(const std::vector<std::string>& elements, const unsigned long& /*unused*/, int numberBase = 0)
        -> unsigned long;

    static auto parse(const std::vector<std::string>& elements, const long& /*unused*/) -> long;

    static auto parse(const std::vector<std::string>& elements, const std::string& /*unused*/) -> std::string;

    template<class T>
    static auto parse(const std::vector<std::string>& elements, const std::vector<T>& /*unused*/) -> std::vector<T>
    {
        const T defval = T();
        std::vector<T> values{};
        std::vector<std::string> buffer(1);

        for(const auto& element : elements)
        {
            buffer[0] = element;
            values.push_back(parse(buffer, defval));
        }

        return values;
    }

    template<typename T>
    static auto parse(const std::vector<std::string>& elements, const numeric_base<T>& wrapper) -> T
    {
        return parse(elements, wrapper.value, 0);
    }

    /// Specialization for number wrapped into numerical base
    /// \tparam T base type of the argument
    /// \tparam base numerical base
    /// \param elements
    /// \param wrapper
    /// \return parsed number
    template<typename T, int base>
    static auto parse(const std::vector<std::string>& elements, const numeric_base<T, base>& wrapper) -> T
    {
        return parse(elements, wrapper.value, wrapper.base);
    }

    template<class T>
    static auto stringify(const T& value) -> std::string
    {
        return std::to_string(value);
    }

    template<class T, int base>
    static auto stringify(const numeric_base<T, base>& wrapper) -> std::string
    {
        return std::to_string(wrapper.value);
    }

    template<class T>
    static auto stringify(const std::vector<T>& values) -> std::string
    {
        std::stringstream ss{};
        ss << "[ ";

        for(const auto& value : values)
        {
            ss << stringify(value) << " ";
        }

        ss << "]";
        return ss.str();
    }

    static auto stringify(const std::string& str) -> std::string;

public:
    explicit parser(int argc, const char** argv);

    explicit parser(int argc, char** argv);

    auto has_help() const -> bool;

    void enable_help();

    void disable_help();

    void reset();

    template<typename T>
    void set_default(bool is_required, const std::string& description = "")
    {
        auto command = std::make_unique<cmd_argument<T>>("", "", description, is_required, false);
        commands_.emplace_back(detail::get_id<cmd_argument<T>>(), std::move(command));
    }

    template<typename T>
    void set_required(const std::string& name,
                      const std::string& alternative,
                      const std::string& description = "",
                      bool dominant = false)
    {
        auto command = std::make_unique<cmd_argument<T>>(name, alternative, description, true, dominant);
        commands_.emplace_back(detail::get_id<cmd_argument<T>>(), std::move(command));
    }

    template<typename T>
    void set_optional(const std::string& name,
                      const std::string& alternative,
                      T defaultValue,
                      const std::string& description = "",
                      bool dominant = false)
    {
        auto command = std::make_unique<cmd_argument<T>>(name, alternative, description, false, dominant);
        command->value = defaultValue;
        commands_.emplace_back(detail::get_id<cmd_argument<T>>(), std::move(command));
    }

    template<typename T>
    void set_callback(const std::string& name,
                      const std::string& alternative,
                      std::function<T(callback_args&)> callback,
                      const std::string& description = "",
                      bool dominant = false)
    {
        auto command = std::make_unique<cmd_function<T>>(name, alternative, description, false, dominant);
        command->callback = callback;
        commands_.emplace_back(detail::get_id<cmd_function<T>>(), std::move(command));
    }

    void run_and_exit_if_error();

    auto run() -> bool;

    auto run(std::ostream& output) -> bool;

    auto run(std::ostream& output, std::ostream& error) -> bool;

    template<typename T>
    auto get(const std::string& name) const -> T
    {
        const std::string alternative_name = "--" + name;
        for(const auto& command_pair : commands_)
        {
            const auto command_type_id = command_pair.first;
            const auto& command = command_pair.second;
            if(command->name == name || command->alternative == alternative_name)
            {
                const auto requested_id = detail::get_id<cmd_argument<T>>();
                if(command_type_id != requested_id)
                {
                    throw std::runtime_error("Invalid usage of the parameter " + name + " detected.");
                }
                else
                {
                    auto cmd = static_cast<cmd_argument<T>*>(command.get());
                    if(cmd)
                    {
                        return cmd->value;
                    }
                }
            }
        }

        throw std::runtime_error("The parameter " + name + " could not be found.");
    }

    template<typename T>
    auto try_get(const std::string& name, T& result) const -> bool
    {
        try
        {
            result = get<T>(name);
            return true;
        }
        catch(const std::exception&)
        {
            return false;
        }
    }

    template<typename T>
    auto get_if(const std::string& name, std::function<T(T)> callback) const -> T
    {
        auto value = get<T>(name);
        return callback(value);
    }

    auto requirements() const -> int;

    auto commands() const -> int;

    auto app_name() const -> const std::string&;

    auto usage() const -> std::string;

protected:
    auto find(const std::string& name) -> cmd_base*;

    auto find_default() -> cmd_base*;

    void print_help(std::stringstream& ss) const;

    auto howto_required(const std::unique_ptr<cmd_base>& command) const -> std::string;

    auto howto_use(const std::unique_ptr<cmd_base>& command) const -> std::string;

    auto no_default() const -> std::string;

private:
    const std::string appname_;
    std::vector<std::string> arguments_;
    std::vector<std::pair<std::uint64_t, std::unique_ptr<cmd_base>>> commands_;
};
} // namespace cmd_line
