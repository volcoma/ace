#pragma once
#include <imgui/imgui.h>

#include <cstdint>

namespace ImGui
{
IMGUI_API bool Splitter(bool split_vertically,
                        float thickness,
                        float* size1,
                        float* size2,
                        float min_size1,
                        float min_size2,
                        float splitter_long_axis_size = -1.0f,
                        float hover_extend = 0.0f);

}
