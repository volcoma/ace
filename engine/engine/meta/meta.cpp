#include "meta.h"
#include <cassert>
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
    gctx = &ctx;
    return true;
}

} // namespace ace
