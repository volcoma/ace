$input v_texcoord0

#include "common.sh"

uniform vec4 u_tonemap;

SAMPLER2D(s_input, 0);


void main()
{
    vec3 color = texture2D(s_input, v_texcoord0).rgb;
    //float lum = clamp(decodeRE8(texture2D(s_texLum, v_texcoord0) ), 0.1, 0.7);
    color = toFilmic(color);
    //color = toGamma(color);

           //vec4 blur = blur9(s_texBlur
           //                  , v_texcoord0
           //                  , v_texcoord1
           //                  , v_texcoord2
           //                  , v_texcoord3
           //                  , v_texcoord4
           //                  );
           //
           //rgb += 0.6 * blur.xyz;

    gl_FragColor = vec4(color, 1.0f);
}
