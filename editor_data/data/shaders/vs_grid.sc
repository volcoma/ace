$input a_position, a_color1
$output v_near_point, v_far_point

#include <bgfx_shader.sh>

// per frame
//uniform vec4 u_camera_position;


vec3 unproject_point (float x, float y, float z)
{
	vec4 unprojected_point = mul(u_invViewProj, vec4 (x, y, z, 1.0));
	
	return unprojected_point.xyz / unprojected_point.w;
}
void main()
{
	v_near_point = unproject_point (a_position.x, a_position.y, 0.0f).xyz;
	v_far_point = unproject_point (a_position.x, a_position.y, 1.0f).xyz;

	gl_Position = vec4 (a_position, 1.0f);
}

