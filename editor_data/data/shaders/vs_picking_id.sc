$input a_position

#include "common.sh"

void main()
{
    vec4 wpos = mul(u_world[0], vec4(a_position, 1.0) );
    gl_Position = mul(u_viewProj, wpos );
}
