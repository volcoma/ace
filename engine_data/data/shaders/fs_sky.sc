$input v_skyColor, v_screenPos, v_viewDir

#include "common.sh"

uniform vec4 	u_parameters; // x - sun size, y - sun bloom, z - exposition, w - time
uniform vec4 	u_sunDirection;
uniform vec4 	u_sunLuminance;

#define u_sun_size u_parameters.x
#define u_sun_bloom u_parameters.y
#define u_exposition u_parameters.z
#define u_time u_parameters.w

// https://www.shadertoy.com/view/4ssXRX
// http://www.loopit.dk/banding_in_games.pdf
// http://www.dspguide.com/ch2/6.htm

//uniformly distributed, normalized rand, [0, 1)
float nrand(in vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

float n4rand_ss(in vec2 n)
{
	float nrnd0 = nrand( n + 0.07*fract( u_time ) );
	float nrnd1 = nrand( n + 0.11*fract( u_time + 0.573953 ) );
	return 0.23*sqrt(-log(nrnd0+0.00001))*cos(2.0*3.141592*nrnd1)+0.5;
}

void main()
{
	float size2 = u_sun_size * u_sun_size;

	vec3 lightDir = normalize(u_sunDirection.xyz);
	float distance = 2.0 * (1.0 - dot(normalize(v_viewDir), lightDir));
	float sun = exp(-distance/ u_sun_bloom / size2) + step(distance, size2);
	float sun2 = min(sun * sun, 1.0);
	vec3 color = v_skyColor + sun2;


    const vec3 u_ground_color = vec3(0.63f, 0.6f, 0.57f);
    const float u_surface_height = 0.99f; // < 1
    vec3 eye_pos = vec3(0.0f, u_surface_height, 0.0f);
    vec3 eye_dir = normalize(v_viewDir);


    float light_angle = dot(-normalize(lightDir), eye_pos);
    vec3 ground_color = (u_ground_color + vec3(1.0f, 1.0f, 1.0f)) * (saturate(-light_angle)) * 0.1f;
    color = mix(color, ground_color, saturate(-eye_dir.y/0.06f + 0.4f));

	//float r = n4rand_ss(v_screenPos);
	//color += vec3(r, r, r) / 40.0;

	gl_FragColor = vec4(color, 1.0);
}
