$input v_texcoord0, v_weye_dir, v_screenPos

#include "common.sh"

uniform vec4 u_parameters;
uniform vec4 u_kr_and_intensity;
uniform vec4 u_turbidity_parameters1;
uniform vec4 u_turbidity_parameters2;
uniform vec4 u_turbidity_parameters3;

#define u_light_direction u_parameters.xyz
#define u_time u_parameters.w

// u_kr, u_intensity
#define u_kr u_kr_and_intensity.xyz
#define u_intensity u_kr_and_intensity.w

// u_rayleigh_strength, u_mie_strength, u_mie_distribution, u_scatter_strength
#define u_rayleigh_strength u_turbidity_parameters1.x
#define u_mie_strength u_turbidity_parameters1.y
#define u_mie_distribution u_turbidity_parameters1.z
#define u_scatter_strength u_turbidity_parameters1.w


// u_rayleigh_brightness, u_mie_brightness, u_spot_brightness, u_spot_distance
#define u_rayleigh_brightness u_turbidity_parameters2.x
#define u_mie_brightness u_turbidity_parameters2.y
#define u_spot_brightness u_turbidity_parameters2.z
#define u_spot_distance u_turbidity_parameters2.w


//u_rayleigh_collection_power, u_mie_collection_power, unused, unused
#define u_rayleigh_collection_power u_turbidity_parameters3.x
#define u_mie_collection_power u_turbidity_parameters3.y


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


float atmospheric_depth(vec3 pos, vec3 dir) 
{
	float a = dot(dir, dir);
	float b = 2.0f * dot(dir, pos);
	float c = dot(pos, pos) - 1.0f;
	float det = b * b - 4.0f * a * c;
	float detSqrt = sqrt(det);
	float q = (-b - detSqrt) / 2.0f;
	float t1 = c / q;
	return t1;
}

float phase(float alpha, float g)
{
	float a = 3.0f * (1.0f - g * g);
	float b = 2.0f * (2.0f + g * g);
	float c = 1.0f + alpha * alpha;
	float d = pow(1.0f + g * g - 2.0f * g * alpha, 1.5f);
	return (a / b) * (c / d);
}

float horizon_extinction(vec3 pos, vec3 dir, float radius)
{
	float u = dot(dir, -pos);
	if(u < 0.0f) 
	{
		return 1.0f;
	}
	vec3 near = pos + u * dir;
	if(length(near) < radius + 0.001f) 
	{
		return 0.0f;
	} 
	else 
	{
		vec3 v2 = normalize(near) * radius - pos;
		float diff = acos(dot(normalize(v2), dir));
		return smoothstep(0.0f, 1.0f, pow(diff * 2.0f, 3.0f));
	}
}

vec3 absorb(vec3 kr, float dist, vec3 color, float factor) 
{
	float f = factor / dist;
	return color - color * pow(kr, vec3(f, f, f));
}

void main()
{
	//const vec3 u_ground_color = vec3(0.63f, 0.6f, 0.57f);	
	const vec3 u_ground_color = vec3(1.369f, 0.349f, 0.341f);
	
	//const vec3 u_kr = vec3(0.12867780436772762f, 0.2478442963618773f, 0.6216065586417131f);
	//const float u_intensity = 1.8f;

	//const float u_rayleigh_strength = 0.139f;
	//const float u_mie_strength = 0.264f;
	//const float u_mie_distribution = 0.53f;
	//const float u_scatter_strength = 0.078;

	//const float u_rayleigh_brightness = 9.0f;
	//const float u_mie_brightness = 0.1f;
	//const float u_spot_brightness = 10.0f;
	//const float u_spot_distance = 300.0f;
	
	
	//const float u_rayleigh_collection_power = 0.81f;
	//const float u_mie_collection_power = 0.39f;
	
	
	const float u_surface_height = 0.99f; // < 1
	const int u_step_count = 2;

	vec3 eye_dir = normalize(v_weye_dir);
	vec3 eye_pos = vec3(0.0f, u_surface_height, 0.0f);

	float alpha = clamp(dot(eye_dir, -u_light_direction), 0, 1);
	float rayleigh_factor = phase(alpha, -0.01) * u_rayleigh_brightness;
	float mie_factor = phase(alpha, u_mie_distribution) * u_mie_brightness;
	float spot = smoothstep(0.0f, u_spot_distance, phase(alpha, 0.9995f)) * u_spot_brightness;
	
	float eye_depth = atmospheric_depth(eye_pos, eye_dir);
	float step_length = eye_depth / float(u_step_count);
	float eye_extinction = horizon_extinction(eye_pos, eye_dir, u_surface_height - 0.05f);

	vec3 rayleigh_collected = vec3(0.0f, 0.0f, 0.0f);
	vec3 mie_collected = vec3(0.0f, 0.0f, 0.0f);
	vec3 mie_collected_raw = vec3(0.0f, 0.0f, 0.0f);

	for(int i = 0; i < u_step_count; ++i) 
	{
		float sample_distance = step_length * float(i);
		vec3 pos = eye_pos + eye_dir * sample_distance;
		float extinction = horizon_extinction(pos, -u_light_direction, u_surface_height - 0.175f);
		float sample_depth = atmospheric_depth(pos, -u_light_direction);
		vec3 influx = absorb(u_kr, sample_depth, vec3(u_intensity, u_intensity, u_intensity), u_scatter_strength);
		vec3 influx_extincted = influx * extinction;
		
		rayleigh_collected += absorb(u_kr, sample_distance, u_kr * influx_extincted, u_rayleigh_strength);
		mie_collected += absorb(u_kr, sample_distance, influx_extincted, u_mie_strength);
		
		mie_collected_raw += absorb(u_kr, sample_distance, influx, u_mie_strength);
	}
    
	mie_collected_raw = ((mie_collected_raw * pow(eye_depth, u_mie_collection_power)) / float(u_step_count)) * 0.05f;

	rayleigh_collected = (rayleigh_collected * eye_extinction * pow(eye_depth, u_rayleigh_collection_power)) / float(u_step_count);
	mie_collected = (mie_collected * eye_extinction * pow(eye_depth, u_mie_collection_power)) / float(u_step_count);

	vec3 spot_color = spot * mie_collected;
	vec3 mie_color = mie_factor * mie_collected;
	vec3 rayleigh_color = rayleigh_factor * rayleigh_collected;

	vec3 color = vec3(mie_color + rayleigh_color);
	float light_angle = dot(-normalize(-u_light_direction), eye_pos);
	
	float factor_ground = saturate(-light_angle) * 0.3f;
	float mix_factor_spot = saturate(-eye_dir.y/0.06f + 1.0f);
	float mix_factor = saturate(-eye_dir.y/0.05f);
	float mix_factor_ground = pow(1.0f - saturate(light_angle), 10.0f);

	vec3 ground_color = u_ground_color * factor_ground;
	ground_color = mix(ground_color, mie_collected_raw, mix_factor_ground);

	color = mix(color, ground_color, mix_factor);
	
	spot_color = mix(spot_color, ground_color, mix_factor_spot);
	color += spot_color;

	//float r = n4rand_ss(v_screenPos);
	//color += vec3(r, r, r) / 40.0;

    gl_FragColor = vec4(color, 1.0f);
}
