$input v_texcoord0

#define SPOT_LIGHT 1
#define SM_SPOT 1
#define SM_VSM 1
#define SM_LINEAR 1

#include "fs_pbr_lighting.sh"

void main()
{
    gl_FragColor = pbr_light(v_texcoord0);
}
