#include "debugdraw.h"

#include "debugdraw.h"

namespace gfx
{
dd_raii::dd_raii(view_id _viewId)
{
    encoder.begin(_viewId);
}

dd_raii::~dd_raii()
{
    encoder.end();
}
} // namespace gfx
