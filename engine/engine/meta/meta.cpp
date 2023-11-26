#include "meta.h"
#include <cassert>
#include <logging/logging.h>

namespace ace
{
namespace
{
rtti::context* gctx{};
}

auto get_app_ctx() -> rtti::context&
{
    assert(gctx);
    return *gctx;
}

auto meta::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    gctx = &ctx;
    return true;
}

auto meta::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    gctx = nullptr;
    return true;
}


} // namespace ace
