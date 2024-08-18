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
#define DIFFUSE_COLOR 1
#define SPECULAR_COLOR 2
#define INDIRECT_SPECULAR_COLOR 3
#define AMBIENT_OCCLUSION 4
#define WORLD_NORMAL 5
#define ROUGHNESS 6
#define METALNESS 7
#define EMISSIVE_COLOR 8
#define SUBSURFACE_COLOR 9
#define DEPTH 10

vec4 gbuffer_visualize(vec2 texcoord0)
{
    GBufferData data = DecodeGBuffer(texcoord0, s_tex0, s_tex1, s_tex2, s_tex3, s_tex4);
    vec3 indirect_specular = texture2D(s_tex5, texcoord0).xyz;

	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	switch(u_mode)
    {
        default:
        case BASE_COLOR:
            color = data.base_color;
            break;
		case DIFFUSE_COLOR:
            color = data.diffuse_color;
            break;
		case SPECULAR_COLOR:
            color = data.specular_color;
            break;
		case INDIRECT_SPECULAR_COLOR:
            color = indirect_specular;
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
            color = vec3_splat(data.depth01);
            break;
    }

    return vec4(color, 1.0f);
}

void main()
{
    gl_FragColor = gbuffer_visualize(v_texcoord0);
}
