#ifndef __PBRLIGHTING_SH__
#define __PBRLIGHTING_SH__

#include "common.sh"
#include "lighting.sh"
#include "shadowmaps/common_shadow.sh"

SAMPLER2D(s_tex0, 0);
SAMPLER2D(s_tex1, 1);
SAMPLER2D(s_tex2, 2);
SAMPLER2D(s_tex3, 3);
SAMPLER2D(s_tex4, 4);
SAMPLER2D(s_tex5, 5); // reflection data
SAMPLER2D(s_tex6, 6); // ibl_brdf_lut

uniform vec4 u_light_position;
uniform vec4 u_light_direction;
uniform vec4 u_light_color_intensity;
uniform vec4 u_light_data;
uniform vec4 u_camera_position;


SAMPLER2D(s_shadowMap0, 7);
SAMPLER2D(s_shadowMap1, 8);
SAMPLER2D(s_shadowMap2, 9);
SAMPLER2D(s_shadowMap3, 10);

uniform vec4 u_params0;
uniform vec4 u_params1;
uniform vec4 u_params2;

uniform vec4 u_smSamplingParams;
uniform vec4 u_csmFarDistances;

#if SM_OMNI
uniform vec4 u_tetraNormalGreen;
uniform vec4 u_tetraNormalYellow;
uniform vec4 u_tetraNormalBlue;
uniform vec4 u_tetraNormalRed;
#endif


uniform mat4 u_lightMtx;
uniform mat4 u_shadowMapMtx0;
uniform mat4 u_shadowMapMtx1;
uniform mat4 u_shadowMapMtx2;
uniform mat4 u_shadowMapMtx3;

#define u_shadowMapBias   u_params1.x
#define u_shadowMapOffset u_params1.y
#define u_shadowMapParam0 u_params1.z
#define u_shadowMapParam1 u_params1.w

#define u_shadowMapShowCoverage u_params2.y
#define u_shadowMapTexelSize    u_params2.z


// Pcf
#define u_shadowMapPcfMode     u_shadowMapParam0
#define u_shadowMapNoiseAmount u_shadowMapParam1

// Vsm
#define u_shadowMapMinVariance     u_shadowMapParam0
#define u_shadowMapDepthMultiplier u_shadowMapParam1

// Esm
#define u_shadowMapHardness        u_shadowMapParam0
#define u_shadowMapDepthMultiplier u_shadowMapParam1

float calculateSlopeBias(float _bias, vec3 _normal, vec3 _lightDir)
{
    return _bias * max(0.0, dot(_normal, _lightDir));
}

float calculateDistanceBias(float _bias, float _distanceFromCamera)
{
    return _bias * (1.0 + _distanceFromCamera * 0.005); // Adjust this factor as necessary
}

float computeVisibility(sampler2D _sampler
                      , vec4 _shadowCoord
                      , float _bias
                      , vec4 _samplingParams
                      , vec2 _texelSize
                      , float _depthMultiplier
                      , float _minVariance
                      , float _hardness
                      , float _distanceFromCamera  // New parameter
                      )
{
    float visibility = 1.0f;

#if SM_LINEAR
    vec4 shadowcoord = vec4(_shadowCoord.xy / _shadowCoord.w, _shadowCoord.z, 1.0);
#else
    vec4 shadowcoord = _shadowCoord;
#endif

    // Adjust bias based on distance
    float adjustedBias = calculateDistanceBias(_bias, _distanceFromCamera);

#if SM_HARD
    visibility = hardShadow(_sampler, shadowcoord, adjustedBias);
#elif SM_PCF
    visibility = PCF(_sampler, shadowcoord, adjustedBias, _samplingParams, _texelSize);
#elif SM_VSM
    visibility = VSM(_sampler, shadowcoord, adjustedBias, _depthMultiplier, _minVariance);
#elif SM_ESM
    visibility = ESM(_sampler, shadowcoord, adjustedBias, _depthMultiplier * _hardness);
#endif

    return visibility;
}


float CalculateSurfaceShadow(vec3 world_position, vec3 world_normal, out vec3 colorCoverage)
{
    float visibility = 1.0f;
    colorCoverage = vec3(0.0f, 0.0f, 0.0f);

#if SM_NOOP
    // No operation
#else
    vec4 wpos = vec4(world_position.xyz + world_normal.xyz * u_shadowMapOffset, 1.0);
#if SM_CSM
    vec4 v_shadowcoord = wpos;
#else
    vec4 v_shadowcoord = mul(u_lightMtx, wpos);
#endif

#if SM_CSM
    vec4 v_texcoord1 = mul(u_shadowMapMtx0, v_shadowcoord);
    vec4 v_texcoord2 = mul(u_shadowMapMtx1, v_shadowcoord);
    vec4 v_texcoord3 = mul(u_shadowMapMtx2, v_shadowcoord);
    vec4 v_texcoord4 = mul(u_shadowMapMtx3, v_shadowcoord);
#elif SM_OMNI
    vec4 v_texcoord1 = mul(u_shadowMapMtx0, v_shadowcoord);
    vec4 v_texcoord2 = mul(u_shadowMapMtx1, v_shadowcoord);
    vec4 v_texcoord3 = mul(u_shadowMapMtx2, v_shadowcoord);
    vec4 v_texcoord4 = mul(u_shadowMapMtx3, v_shadowcoord);
#endif

#if SM_LINEAR
    v_shadowcoord.z += 0.5;
#if SM_CSM
    v_texcoord1.z += 0.5;
    v_texcoord2.z += 0.5;
    v_texcoord3.z += 0.5;
    v_texcoord4.z += 0.5;
#elif SM_OMNI
    v_texcoord1.z += 0.5;
    v_texcoord2.z += 0.5;
    v_texcoord3.z += 0.5;
    v_texcoord4.z += 0.5;
#endif

#endif

    // Calculate distance from the camera to the fragment
    float distanceFromCamera = length(u_camera_position.xyz - world_position);

#if SM_CSM
    vec2 texelSize = vec2_splat(u_shadowMapTexelSize);

    vec2 texcoord1 = v_texcoord1.xy/v_texcoord1.w;
    vec2 texcoord2 = v_texcoord2.xy/v_texcoord2.w;
    vec2 texcoord3 = v_texcoord3.xy/v_texcoord3.w;
    vec2 texcoord4 = v_texcoord4.xy/v_texcoord4.w;

    bool selection0 = all(lessThan(texcoord1, vec2_splat(0.99999))) && all(greaterThan(texcoord1, vec2_splat(0.00001)));
    bool selection1 = all(lessThan(texcoord2, vec2_splat(0.99999))) && all(greaterThan(texcoord2, vec2_splat(0.00001)));
    bool selection2 = all(lessThan(texcoord3, vec2_splat(0.99999))) && all(greaterThan(texcoord3, vec2_splat(0.00001)));
    bool selection3 = all(lessThan(texcoord4, vec2_splat(0.99999))) && all(greaterThan(texcoord4, vec2_splat(0.00001)));

    if (selection0)
    {
        vec4 shadowcoord = v_texcoord1;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
        colorCoverage = vec3(-coverage, coverage, -coverage);
        visibility = computeVisibility(s_shadowMap0
                        , shadowcoord
                        , u_shadowMapBias
                        , u_smSamplingParams
                        , texelSize
                        , u_shadowMapDepthMultiplier
                        , u_shadowMapMinVariance
                        , u_shadowMapHardness
                        , distanceFromCamera  // Pass the distance
                        );
    }
    else if (selection1)
    {
        vec4 shadowcoord = v_texcoord2;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
        colorCoverage = vec3(coverage, coverage, -coverage);
        visibility = computeVisibility(s_shadowMap1
                        , shadowcoord
                        , u_shadowMapBias
                        , u_smSamplingParams
                        , texelSize/2.0
                        , u_shadowMapDepthMultiplier
                        , u_shadowMapMinVariance
                        , u_shadowMapHardness
                        , distanceFromCamera  // Pass the distance
                        );
    }
    else if (selection2)
    {
        vec4 shadowcoord = v_texcoord3;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
        colorCoverage = vec3(-coverage, -coverage, coverage);
        visibility = computeVisibility(s_shadowMap2
                        , shadowcoord
                        , u_shadowMapBias
                        , u_smSamplingParams
                        , texelSize/3.0
                        , u_shadowMapDepthMultiplier
                        , u_shadowMapMinVariance
                        , u_shadowMapHardness
                        , distanceFromCamera  // Pass the distance
                        );
    }
    else // selection3
    {
        vec4 shadowcoord = v_texcoord4;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.4;
        colorCoverage = vec3(coverage, -coverage, -coverage);
        visibility = computeVisibility(s_shadowMap3
                        , shadowcoord
                        , u_shadowMapBias
                        , u_smSamplingParams
                        , texelSize/4.0
                        , u_shadowMapDepthMultiplier
                        , u_shadowMapMinVariance
                        , u_shadowMapHardness
                        , distanceFromCamera  // Pass the distance
                        );
    }
#elif SM_OMNI
    vec2 texelSize = vec2_splat(u_shadowMapTexelSize/4.0);

    vec4 faceSelection;
    vec3 pos = v_shadowcoord.xyz;
    faceSelection.x = dot(u_tetraNormalGreen.xyz,  pos);
    faceSelection.y = dot(u_tetraNormalYellow.xyz, pos);
    faceSelection.z = dot(u_tetraNormalBlue.xyz,   pos);
    faceSelection.w = dot(u_tetraNormalRed.xyz,    pos);

    vec4 shadowcoord;
    float faceMax = max(max(faceSelection.x, faceSelection.y), max(faceSelection.z, faceSelection.w));
    if (faceSelection.x == faceMax)
    {
        shadowcoord = v_texcoord1;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
        colorCoverage = vec3(-coverage, coverage, -coverage);
    }
    else if (faceSelection.y == faceMax)
    {
        shadowcoord = v_texcoord2;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
        colorCoverage = vec3(coverage, coverage, -coverage);
    }
    else if (faceSelection.z == faceMax)
    {
        shadowcoord = v_texcoord3;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
        colorCoverage = vec3(-coverage, -coverage, coverage);
    }
    else // (faceSelection.w == faceMax)
    {
        shadowcoord = v_texcoord4;

        float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
        colorCoverage = vec3(coverage, -coverage, -coverage);
    }

    visibility = computeVisibility(s_shadowMap0
                    , shadowcoord
                    , u_shadowMapBias
                    , u_smSamplingParams
                    , texelSize
                    , u_shadowMapDepthMultiplier
                    , u_shadowMapMinVariance
                    , u_shadowMapHardness
                    , distanceFromCamera  // Pass the distance
                    );
#else
    vec2 texelSize = vec2_splat(u_shadowMapTexelSize);

    vec4 shadowcoord = v_shadowcoord;

    float coverage = texcoordInRange(shadowcoord.xy/shadowcoord.w) * 0.3;
    colorCoverage = vec3(coverage, -coverage, -coverage);

    visibility = computeVisibility(s_shadowMap0
                    , shadowcoord
                    , u_shadowMapBias
                    , u_smSamplingParams
                    , texelSize
                    , u_shadowMapDepthMultiplier
                    , u_shadowMapMinVariance
                    , u_shadowMapHardness
                    , distanceFromCamera  // Pass the distance
                    );
#endif
#endif

    return visibility;
}

vec4 pbr_light(vec2 texcoord0)
{
    GBufferData data = decodeGBuffer(texcoord0, s_tex0, s_tex1, s_tex2, s_tex3, s_tex4);
    vec3 indirect_specular = texture2D(s_tex5, texcoord0).xyz;
    vec3 clip = vec3(texcoord0 * 2.0 - 1.0, data.depth);
    clip = clipTransform(clip);
    vec3 world_position = clipToWorld(u_invViewProj, clip);
    vec3 lobe_roughness = vec3(0.0f, data.roughness, 1.0f);
    vec3 light_color = u_light_color_intensity.xyz;
    float intensity = u_light_color_intensity.w;
    vec3 specular_color = mix( 0.04f * light_color, data.base_color, data.metalness );
    vec3 albedo_color = data.base_color - data.base_color * data.metalness;

    albedo_color *= data.ambient_occlusion;
    specular_color *= data.ambient_occlusion;

#if DIRECTIONAL_LIGHT
    vec3 vector_to_light = -u_light_direction.xyz;
    vec3 indirect_diffuse = albedo_color * 0.1f;
#else
    vec3 vector_to_light = u_light_position.xyz - world_position;
    vec3 indirect_diffuse = vec3(0.0f, 0.0f, 0.0f);
#endif
    float distance_sqr = dot( vector_to_light, vector_to_light );
    vec3 N = data.world_normal;
    vec3 V = normalize(u_camera_position.xyz - world_position);
    vec3 L = vector_to_light / sqrt( distance_sqr );
    float NoL = saturate( dot(N, L) );
    float distance_attenuation = 1.0f;

#if POINT_LIGHT
    vec3 vector_to_light_over_radius = vector_to_light / u_light_data.x;
    float light_radius_mask = RadialAttenuation(vector_to_light_over_radius, u_light_data.y);
    float light_falloff = 1.0f;
#elif SPOT_LIGHT
    vec3 vector_to_light_over_radius = vector_to_light / u_light_data.x;
    float light_radius_mask = RadialAttenuation(vector_to_light_over_radius, 1.0f);
    float light_falloff = SpotAttenuation( vector_to_light_over_radius, normalize(u_light_direction.xyz), vec2(u_light_data.z, 1.0f / (u_light_data.y - u_light_data.z )));
#else
    float light_radius_mask = 1.0f;
    float light_falloff = 1.0f;
#endif


    vec3 colorCoverage = vec3(0.0f, 0.0f, 0.0f);
    float surface_shadow = CalculateSurfaceShadow(world_position, N, colorCoverage);
    float subsurface_shadow = 1.0f;
    float surface_attenuation = (intensity * distance_attenuation * light_radius_mask * light_falloff) * surface_shadow;
    float subsurface_attenuation = (distance_attenuation * light_radius_mask * light_falloff) * subsurface_shadow;

    vec3 energy = AreaLightSpecular(0.0f, 0.0f, normalize(vector_to_light), lobe_roughness, vector_to_light, L, V, N);
    SurfaceShading surface_lighting = StandardShading(albedo_color, indirect_diffuse, specular_color, indirect_specular, s_tex6, lobe_roughness, energy, data.metalness, data.ambient_occlusion, L, V, N);
    vec3 direct_surface_lighting = surface_lighting.direct;
    vec3 indirect_surface_lighting = surface_lighting.indirect;
    //vec3 subsurface_lighting = SubsurfaceShadingTwoSided(data.subsurface_color, L, V, N);
    vec3 subsurface_lighting = SubsurfaceShading(data.subsurface_color, data.subsurface_opacity, data.ambient_occlusion, L, V, N);
    vec3 surface_multiplier = light_color * (NoL * surface_attenuation);
    vec3 subsurface_multiplier = (light_color * subsurface_attenuation);

    vec3 lighting = surface_multiplier * direct_surface_lighting + (subsurface_lighting + indirect_surface_lighting) * subsurface_multiplier + data.emissive_color + colorCoverage * u_shadowMapShowCoverage;

    vec4 result;
    result.xyz = lighting;
    result.w = 1.0f;
    return result;
}

#endif // __PBRLIGHTING_SH__
