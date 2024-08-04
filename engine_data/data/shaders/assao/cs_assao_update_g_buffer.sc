/*
 * Copyright 2018 Attila Kocsis. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "bgfx_compute.sh" 
#include "uniforms.sh"

IMAGE2D_RW(rgba8Image, rgba8, 0);
IMAGE2D_RO(r8Image, r8, 1);


NUM_THREADS(8, 8, 1)
void main() 
{
    // Get the current pixel position
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    // Read the current RGBA color
    vec4 color = imageLoad(rgba8Image, pixelCoord);

    // Read the R8 value (use as alpha)
    float alpha = imageLoad(r8Image, pixelCoord).r;

    // Update the alpha channel while preserving RGB
    color *= alpha;

    // Write the updated color back to the RGBA8 image
    imageStore(rgba8Image, pixelCoord, color);
}
