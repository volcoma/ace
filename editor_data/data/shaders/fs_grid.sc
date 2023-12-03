$input v_near_point
$input v_far_point

#include <bgfx_shader.sh>

uniform vec4 u_params;

#define u_grid_height   u_params.x
#define u_camera_near   u_params.y
#define u_camera_far    u_params.z

vec4 grid (vec3 frag_position_3d, float scale, float thickness, float axis_alpha)
{
	// dont want the grid to be infinite?
	// 	uncomment this bit, set your boundaries to whatever you want
	//if (frag_position_3d.x > 10.0f
	//	|| frag_position_3d.x < -10.0f
	//	|| frag_position_3d.z > 10.0f
	//	|| frag_position_3d.z < -10.0f)
	//{
	//	return vec4 (0.0f, 0.0f, 0.0f, 0.0f);
	//}

	vec2 coord = frag_position_3d.xz * scale;
	vec2 derivative = fwidth (coord);
    vec2 fr = fract(coord - vec2(0.5f, 0.5f));
    vec2 frabs = abs(fr);
	vec2 gr = frabs / derivative;
	float ln = min(gr.x, gr.y);
	float minimum_z = min(derivative.y, 1.0f);
	float minimum_x = min(derivative.x, 1.0f);
    
    float opacity = 0.3f;
    float axisLineThreshold = thickness / scale;
	vec4 color = vec4(1.0f, 1.0f, 1.0f, thickness - min(ln, thickness));

    color.a *= opacity;
	// z axis color
	if (frag_position_3d.x > -axisLineThreshold * minimum_x
		&& frag_position_3d.x < axisLineThreshold * minimum_x)
	{
        color.r = 0.1f;
        color.g = 1.0f;
        color.b = 0.3f;
        color.a = axis_alpha;
	}

	// x axis color
	if (frag_position_3d.z > -axisLineThreshold * minimum_z
		&& frag_position_3d.z < axisLineThreshold * minimum_z)
	{
		color.r = 1.0f;
        color.g = 0.35f;
        color.b = 0.3f;
        color.a = axis_alpha;
	}
	
	return color;
}

float compute_depth (vec3 position, in mat4 viewProj)
{
	vec4 clip_space_position = mul(viewProj, vec4 (position.xyz, 1.0));
    return (clip_space_position.z / clip_space_position.w);
	// the depth calculation in the original article is for vulkan
	// the depth calculation for opengl is:
	// 	(far - near) * 0.5f * ndc_depth + (far + near) * 0.5f
	// 	far = 1.0f  (opengl max depth)
	// 	near = 0.0f  (opengl min depth)
	//		ndc_depth = clip_space_position.z / clip_space_position.w
	//	since our near and far are fixed, we can reduce the above formula to the following
	// return 0.5f + 0.5f * (clip_space_position.z / clip_space_position.w);
	// this could also be (ndc_depth + 1.0f) * 0.5f
}

float compute_linear_depth (vec3 position, in mat4 viewProj)
{
	float near = u_camera_near;
	float far = u_camera_far;
	vec4 clip_space_position = mul(viewProj, vec4 (position.xyz, 1.0f));
	float clip_space_depth = (clip_space_position.z / clip_space_position.w) * 2.0f - 1.0f;
	float linear_depth = (2.0f * near * far) / (far + near - clip_space_depth * (far - near));

    // normalize
	return linear_depth / far;
}

void main()
{
	float t = (u_grid_height - v_near_point.y) / (v_far_point.y - v_near_point.y);
	vec3 frag_position_3d = v_near_point + t * (v_far_point - v_near_point);

	gl_FragDepth = compute_depth (frag_position_3d, u_viewProj);

	float linear_depth = compute_linear_depth (frag_position_3d, u_viewProj);
	float fading = max (0, (0.5 - linear_depth));

    vec4 color = (grid (frag_position_3d, 0.05f, 5.0f * fading, 0.7f) + grid(frag_position_3d, 1.0f, 3.0f * fading, 0.0f)) * float(t > 0.0f);
    color.a *= fading;
	gl_FragColor = color;
}