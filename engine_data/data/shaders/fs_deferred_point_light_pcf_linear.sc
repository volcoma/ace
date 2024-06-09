$input v_texcoord0

#define POINT_LIGHT 1
#define SM_OMNI 1
#define SM_PCF 1
#define SM_LINEAR 1

#include "fs_pbr_lighting.sh"

void main()
{
    gl_FragColor = pbr_light(v_texcoord0);
}
