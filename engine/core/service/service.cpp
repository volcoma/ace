#include "service.h"
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;

service::service(int argc, char* argv[]) : parser_(argc, argv)
{
}

auto service::load(const module_desc& desc) -> bool
{
    rttr::type::get<rtti::context>();

    std::cout << "service::" << __func__ << " module " << desc.lib_name << std::endl;
    module_data module;
    module.desc = desc;
    module.plugin = std::make_unique<rttr::library>(module.desc.lib_name);

    if(!module.plugin->load())
    {
        // std::cerr << module.plugin->get_error_string() << std::endl;
        // return false;
    }

    {
        auto type = rttr::type::get_by_name(module.desc.type_name);
        if(!type.invoke("create", {}, {ctx_, parser_}).to_bool())
        {
            return false;
        }
    }

    modules_.emplace_back(std::move(module));

    return true;
}

auto service::unload(const module_data& module) -> bool
{
    std::cout << "service::" << __func__ << " module " << module.desc.lib_name << std::endl;

    auto type = rttr::type::get_by_name(module.desc.type_name);

    if(!type.invoke("deinit", {}, {}).to_bool())
    {
        return false;
    }

    if(!type.invoke("destroy", {}, {}).to_bool())
    {
        return false;
    }

    if(!module.plugin->unload())
    {
        std::cerr << module.plugin->get_error_string() << std::endl;
        return false;
    }

    return true;
}

auto service::load(const std::vector<module_desc>& descs) -> bool
{
    bool batch = true;
    for(const auto& desc : descs)
    {
        batch &= load(desc);
    }

    if(batch)
    {
        batch &= init();
    }

    if(!batch)
    {
        unload();
    }

    return batch;
}

auto service::unload() -> bool
{
    bool batch = true;
    for(auto it = std::rbegin(modules_); it != std::rend(modules_); ++it)
    {
        auto& module = *it;
        batch &= unload(module);
    }

    modules_.clear();
    return batch;
}

auto service::init() -> bool
{
    std::stringstream out, err;
    if(!parser_.run(out, err))
    {
        auto parse_error = out.str();
        if(parse_error.empty())
        {
            parse_error = "Failed to parse command line.";
        }
        //		APPLOG_ERROR(parse_error);

        return false;
    }
    auto parse_info = out.str();
    if(!parse_info.empty())
    {
        //		APPLOG_INFO(parse_info);
    }

    for(const auto& module : modules_)
    {
        auto type = rttr::type::get_by_name(module.desc.type_name);

        if(!type.invoke("init", {}, {parser_}).to_bool())
        {
            return false;
        }
    }

    parser_.reset();

    return true;
}

auto service::process() -> bool
{
    //    std::cout << "service::" << __func__ << std::endl;
    bool processed = false;
    for(const auto& module : modules_)
    {
        auto type = rttr::type::get_by_name(module.desc.type_name);

        if(!type.invoke("process", {}, {}).to_bool())
        {
            return false;
        }

        processed = true;
    }

    return processed;
}

auto service::get_cmd_line_parser() -> cmd_line::parser&
{
    return parser_;
}


int service_main(const char* name, int argc, char* argv[])
{
    std::vector<module_desc> modules{{name, name}};

    service app(argc, argv);

    // for(int i = 0; i < 3; ++i)
    {

        if(!app.load(modules))
        {
            return -1;
        }

        bool run = true;

        while(run)
        {
            run = app.process();
        }

        if(!app.unload())
        {
            return -1;
        }
    }
    return 0;
}
