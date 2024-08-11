$input v_texcoord0

#include "../common.sh"
#include "tonemapping.sh"

uniform vec4 u_tonemap;

SAMPLER2D(s_input, 0);

#define TONEMAP_NONE 0
#define TONEMAP_EXPONENTIAL 1
#define TONEMAP_REINHARD 2
#define TONEMAP_REINHARD_LUM 3
#define TONEMAP_HABLE 4
#define TONEMAP_DUIKER 5
#define TONEMAP_ACES 6
#define TONEMAP_ACES_LUM 7
#define TONEMAP_FILMIC 8

#define u_tonemappingExposure u_tonemap.x
#define u_tonemappingMode int(u_tonemap.y)


void main()
{
    vec3 color = texture2D(s_input, v_texcoord0).rgb;

           //vec4 blur = blur9(s_texBlur
           //                  , v_texcoord0
           //                  , v_texcoord1
           //                  , v_texcoord2
           //                  , v_texcoord3
           //                  , v_texcoord4
           //                  );
           //
           //color += 0.6 * blur.xyz;

    color *= u_tonemappingExposure;

    switch(u_tonemappingMode)
    {
        default:
        case TONEMAP_NONE:
            color = saturate(color);
            break;
        case TONEMAP_EXPONENTIAL:
            color = linear_to_srgb(tonemap_exponential(color));
            break;
        case TONEMAP_REINHARD:
            color = linear_to_srgb(tonemap_reinhard(color));
            break;
        case TONEMAP_REINHARD_LUM:
            color = linear_to_srgb(tonemap_reinhard_luminance(color));
            break;
        case TONEMAP_HABLE:
            color = linear_to_srgb(tonemap_hable(color));
            break;
        case TONEMAP_DUIKER:
            color = linear_to_srgb(tonemap_duiker(color));
            break;
        case TONEMAP_ACES:
            color = linear_to_srgb(tonemap_aces(color));
            break;
        case TONEMAP_ACES_LUM:
            color = linear_to_srgb(tonemap_aces_luminance(color));
            break;
        case TONEMAP_FILMIC:
            color = toFilmic(color);
            break;
    }


    gl_FragColor = vec4(color, 1.0f);
}
