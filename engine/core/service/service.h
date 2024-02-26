#pragma once

#include <context/context.hpp>
#include <rttr/type>
#include <cmd_line/parser.h>

struct module_desc
{
    std::string lib_name;
    std::string type_name;
};

struct module_data
{
    module_desc desc;
    std::unique_ptr<rttr::library> plugin;
};

struct service
{
    service(int argc, char* argv[]);
    auto load(const module_desc& desc) -> bool;
    auto unload(const module_data& module) -> bool;

    auto load(const std::vector<module_desc>& descs) -> bool;
    auto unload() -> bool;

    auto init() -> bool;
    auto process() -> bool;

    auto get_cmd_line_parser() -> cmd_line::parser&;

private:
    rtti::context ctx_;
    cmd_line::parser parser_;
    std::vector<module_data> modules_;
};

int service_main(const char* name, int argc, char* argv[]);
