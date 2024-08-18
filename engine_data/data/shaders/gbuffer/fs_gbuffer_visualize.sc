$input v_texcoord0

#include "../common.sh"
#include "../lighting.sh"

SAMPLER2D(s_tex0, 0);
SAMPLER2D(s_tex1, 1);
SAMPLER2D(s_tex2, 2);
SAMPLER2D(s_tex3, 3);
SAMPLER2D(s_tex4, 4);
SAMPLER2D(s_tex5, 5);

uniform vec4 u_params;

#define u_mode int(u_params.x)

#define BASE_COLOR 0
#define AMBIENT_OCCLUSION 1
#define WORLD_NORMAL 2
#define ROUGHNESS 3
#define METALNESS 4
#define EMISSIVE_COLOR 5
#define SUBSURFACE_COLOR 6
#define DEPTH 7

vec4 gbuffer_visualize(vec2 texcoord0)
{
    GBufferData data = decodeGBuffer(texcoord0, s_tex0, s_tex1, s_tex2, s_tex3, s_tex4);
   
	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	switch(u_mode)
    {
        default:
        case BASE_COLOR:
            color = data.base_color;
            break;
        case AMBIENT_OCCLUSION:
            color = vec3_splat(data.ambient_occlusion);
            break;
        case WORLD_NORMAL:
            color = data.world_normal;
            break;
        case ROUGHNESS:
            color = vec3_splat(data.roughness);
            break;
        case METALNESS:
            color = vec3_splat(data.metalness);
            break;
        case EMISSIVE_COLOR:
            color = data.emissive_color;
            break;
        case SUBSURFACE_COLOR:
            color = data.subsurface_color;
            break;
        case DEPTH:
            color = vec3_splat(data.depth);
            break;
    }

    return vec4(color, 1.0f);
}

void main()
{
    gl_FragColor = gbuffer_visualize(v_texcoord0);
}
