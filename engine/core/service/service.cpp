#include "service.h"
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

service::service(int argc, char* argv[])
    : parser_(argc, argv)
{

}

auto service::load(const module_desc &desc) -> bool
{
    rttr::type::get<rtti::context>();

    std::cout << "service::" << __func__ << " module " << desc.lib_name << std::endl;
    module_data module;
    module.desc = desc;
    module.plugin = std::make_unique<rttr::library>(module.desc.lib_name);

    if(!module.plugin->load())
    {
        std::cerr << module.plugin->get_error_string() << std::endl;
        return false;
    }

    {
        auto type = rttr::type::get_by_name(module.desc.type_name);
        type.invoke("create", {}, std::vector<rttr::argument>{ctx_, parser_});
    }

    modules_.emplace_back(std::move(module));

    return true;
}

auto service::unload(const module_data &module) -> bool
{
    std::cout << "service::" << __func__ << " module " << module.desc.lib_name << std::endl;

    auto type = rttr::type::get_by_name(module.desc.type_name);

    type.invoke("deinit", {}, {ctx_});

    if(!module.plugin->unload())
    {
        std::cerr << module.plugin->get_error_string() << std::endl;
        return false;
    }

    return true;
}

auto service::load(const std::vector<module_desc> &descs) -> bool
{
    bool batch = true;
    for(const auto& desc : descs)
    {
        batch &= load(desc);
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
        auto process = type.get_method("init");

        if(process)
        {
            if(!process.invoke({}, {ctx_}, {parser_}).to_bool())
            {
                return false;
            }
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
        auto process = type.get_method("process");

        if(process)
        {
            if(!process.invoke({}, {ctx_}).to_bool())
            {
                return false;
            }

            processed = true;
        }
    }

    return processed;
}
