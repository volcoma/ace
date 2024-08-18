$input v_wpos, v_pos, v_wnormal, v_wtangent, v_wbitangent, v_texcoord0

#include "common.sh"
#include "lighting.sh"

SAMPLER2D(s_tex_color,  0);
SAMPLER2D(s_tex_normal, 1);
SAMPLER2D(s_tex_roughness, 2);
SAMPLER2D(s_tex_metalness, 3);
SAMPLER2D(s_tex_ao, 4);
SAMPLER2D(s_tex_emissive, 5);

// per frame
uniform vec4 u_camera_wpos;
uniform vec4 u_camera_clip_planes; //.x = near, .y = far

// per instance
uniform vec4 u_base_color;
uniform vec4 u_subsurface_color;
uniform vec4 u_emissive_color;
uniform vec4 u_surface_data;
uniform vec4 u_surface_data2;

uniform vec4 u_tiling;
uniform vec4 u_dither_threshold; //.x = alpha threshold .y = distance threshold
uniform vec4 u_lod_params;

#define u_surface_roughness u_surface_data.x
#define u_surface_metalness u_surface_data.y
#define u_surface_bumpiness u_surface_data.z
#define u_surface_alpha_test_value u_surface_data.w
#define u_surface_metalness_roughness_combined u_surface_data2.x

#define u_camear_near u_camera_clip_planes.x
#define u_camear_far u_camera_clip_planes.y

#define u_dither_alpha_threshold u_dither_threshold.x
#define u_dither_distance_threshold u_dither_threshold.y


void main()
{
	vec2 texcoords = v_texcoord0.xy * u_tiling.xy;

	vec4 metalness_val = texture2D(s_tex_metalness, texcoords);
	float metalness =  u_surface_metalness;
	
	float roughness = u_surface_roughness;
	if(u_surface_metalness_roughness_combined > 0.5f)
	{
	
		roughness *= metalness_val.g;
		metalness *= metalness_val.b;
	}
	else
	{
		roughness *= texture2D(s_tex_roughness, texcoords).r;
		metalness *= metalness_val.r;
	}
	
	roughness = clamp(roughness, 0.05f, 1.0f);
	
	float ambient_occlusion = texture2D(s_tex_ao, texcoords).r;
	vec3 emissive = texture2D(s_tex_emissive, texcoords).rgb;

	float bumpiness = u_surface_bumpiness;
	float alpha_test_value = u_surface_alpha_test_value;

	vec3 view_direction = u_camera_wpos.xyz - v_wpos;
	vec3 tangent_space_normal = getTangentSpaceNormal( s_tex_normal, texcoords, bumpiness );

	mat3 tangent_to_world_space = computeTangentToWorldSpaceMatrix(normalize(v_wnormal), normalize(view_direction), texcoords.xy);
	//mat3 tangent_to_world_space = constructTangentToWorldSpaceMatrix(normalize(v_wtangent), normalize(v_wbitangent), normalize(v_wnormal));

	vec3 wnormal = normalize( mul( tangent_to_world_space, tangent_space_normal ).xyz );
	vec4 albedo_color = texture2D(s_tex_color, texcoords) * u_base_color;

	float distance = length(view_direction) - u_camear_near * 2.0f;
	float distance_factor = saturate(distance / u_dither_distance_threshold);
	float dither = dither16x16(gl_FragCoord.xy);

	if((albedo_color.a + (dither * (1.0f - alpha_test_value)) < 1.0f) ||
	(distance_factor + dither < 1.0f) ||
	(u_lod_params.x - dither * u_lod_params.y) > u_lod_params.z)
	{
		discard;
	}

	GBufferData buffer;
	buffer.base_color = albedo_color.rgb;
	buffer.ambient_occlusion = ambient_occlusion;
	buffer.world_normal = wnormal;
	buffer.roughness = roughness;
	buffer.emissive_color = emissive * u_emissive_color.rgb;
	buffer.metalness = metalness;
	buffer.subsurface_color = u_subsurface_color.rgb;
	buffer.subsurface_opacity = u_subsurface_color.w;

	vec4 result[4];
    EncodeGBuffer(buffer, result);

	gl_FragData[0] = result[0];
	gl_FragData[1] = result[1];
	gl_FragData[2] = result[2];
	gl_FragData[3] = result[3];
}
