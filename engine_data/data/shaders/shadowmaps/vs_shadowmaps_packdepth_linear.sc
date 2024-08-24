$input a_position
$output v_depth

/*
 * Copyright 2013-2014 Dario Manesku. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "../common.sh"

void main()
{
    vec4 wpos = mul(u_world[0], vec4(a_position, 1.0) );
    gl_Position = mul(u_viewProj, wpos );
	v_depth = gl_Position.z * 0.5 + 0.5;
}
