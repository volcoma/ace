/*
 * Copyright 2013-2014 Dario Manesku. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "../common.sh"

vec2 samplePoisson(int index)
{

CONST(vec2) PoissonDistribution[64] =
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_SPIRV
{
#else
vec2[](
#endif
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790),
    vec2(-0.413923, -0.439757),
    vec2(-0.979153, -0.201245),
    vec2(-0.865579, -0.288695),
    vec2(-0.243704, -0.186378),
    vec2(-0.294920, -0.055748),
    vec2(-0.604452, -0.544251),
    vec2(-0.418056, -0.587679),
    vec2(-0.549156, -0.415877),
    vec2(-0.238080, -0.611761),
    vec2(-0.267004, -0.459702),
    vec2(-0.100006, -0.229116),
    vec2(-0.101928, -0.380382),
    vec2(-0.681467, -0.700773),
    vec2(-0.763488, -0.543386),
    vec2(-0.549030, -0.750749),
    vec2(-0.809045, -0.408738),
    vec2(-0.388134, -0.773448),
    vec2(-0.429392, -0.894892),
    vec2(-0.131597, 0.065058),
    vec2(-0.275002, 0.102922),
    vec2(-0.106117, -0.068327),
    vec2(-0.294586, -0.891515),
    vec2(-0.629418, 0.379387),
    vec2(-0.407257, 0.339748),
    vec2(0.071650, -0.384284),
    vec2(0.022018, -0.263793),
    vec2(0.003879, -0.136073),
    vec2(-0.137533, -0.767844),
    vec2(-0.050874, -0.906068),
    vec2(0.114133, -0.070053),
    vec2(0.163314, -0.217231),
    vec2(-0.100262, -0.587992),
    vec2(-0.004942, 0.125368),
    vec2(0.035302, -0.619310),
    vec2(0.195646, -0.459022),
    vec2(0.303969, -0.346362),
    vec2(-0.678118, 0.685099),
    vec2(-0.628418, 0.507978),
    vec2(-0.508473, 0.458753),
    vec2(0.032134, -0.782030),
    vec2(0.122595, 0.280353),
    vec2(-0.043643, 0.312119),
    vec2(0.132993, 0.085170),
    vec2(-0.192106, 0.285848),
    vec2(0.183621, -0.713242),
    vec2(0.265220, -0.596716),
    vec2(-0.009628, -0.483058),
    vec2(-0.018516, 0.435703)
#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_SPIRV
};
#else
);
#endif

    return PoissonDistribution[int(mod(index,64))];
}

float texcoordInRange(vec2 _texcoord)
{
    bool inRange = all(greaterThan(_texcoord, vec2_splat(0.0)))
                && all(lessThan   (_texcoord, vec2_splat(1.0)))
                 ;

    return float(inRange);
}

float hardShadowLod(sampler2D _sampler, float lod, vec4 _shadowCoord, float _bias)
{
    vec2 texCoord = _shadowCoord.xy/_shadowCoord.w;

    bool outside = any(greaterThan(texCoord, vec2_splat(1.0)))
                || any(lessThan   (texCoord, vec2_splat(0.0)))
                 ;

    if (outside)
    {
        return 1.0;
    }

    float receiver = (_shadowCoord.z-_bias)/_shadowCoord.w;
    float occluder = unpackRgbaToFloat(texture2DLod(_sampler, texCoord, lod) );

    float visibility = step(receiver, occluder);
    return visibility;
}

float hardShadow(sampler2D _sampler, vec4 _shadowCoord, float _bias)
{
    return hardShadowLod(_sampler, 0.0, _shadowCoord, _bias);
}


float PCFLodOffset(sampler2D _sampler, float lod, vec2 offset, vec4 _shadowCoord, float _bias, vec4 _pcfParams, vec2 _texelSize)
{
    float result = 0.0;

    for ( int i = 0; i < 16; ++i )
    {
        vec2 jitteredOffset = samplePoisson(i) * offset;
        result += hardShadowLod(_sampler, lod, _shadowCoord + vec4(jitteredOffset, 0.0, 0.0), _bias);
    }
    return result / 16;
}

float PCFLod(sampler2D _sampler, float lod, vec2 filterRadius, vec4 _shadowCoord, float _bias, vec4 _pcfParams, vec2 _texelSize)
{
    vec2 offset = filterRadius * _pcfParams.zw * _texelSize * _shadowCoord.w;

    return PCFLodOffset(_sampler, lod, offset, _shadowCoord, _bias, _pcfParams, _texelSize);}

float PCF(sampler2D _sampler, vec4 _shadowCoord, float _bias, vec4 _pcfParams, vec2 _texelSize)
{
    return PCFLod(_sampler, 0.0, vec2(2.0, 2.0), _shadowCoord, _bias, _pcfParams, _texelSize);
}

float VSM(sampler2D _sampler, vec4 _shadowCoord, float _bias, float _depthMultiplier, float _minVariance)
{
    vec2 texCoord = _shadowCoord.xy/_shadowCoord.w;

    bool outside = any(greaterThan(texCoord, vec2_splat(1.0)))
                || any(lessThan   (texCoord, vec2_splat(0.0)))
                 ;

    if (outside)
    {
        return 1.0;
    }

    float receiver = (_shadowCoord.z-_bias)/_shadowCoord.w * _depthMultiplier;
    vec4 rgba = texture2D(_sampler, texCoord);
    vec2 occluder = vec2(unpackHalfFloat(rgba.rg), unpackHalfFloat(rgba.ba)) * _depthMultiplier;

    if (receiver < occluder.x)
    {
        return 1.0;
    }

    float variance = max(occluder.y - (occluder.x*occluder.x), _minVariance);
    float d = receiver - occluder.x;

    float visibility = variance / (variance + d*d);

    return visibility;
}

float ESM(sampler2D _sampler, vec4 _shadowCoord, float _bias, float _depthMultiplier)
{
    vec2 texCoord = _shadowCoord.xy/_shadowCoord.w;

    bool outside = any(greaterThan(texCoord, vec2_splat(1.0)))
                || any(lessThan   (texCoord, vec2_splat(0.0)))
                 ;

    if (outside)
    {
        return 1.0;
    }

    float receiver = (_shadowCoord.z-_bias)/_shadowCoord.w;
    float occluder = unpackRgbaToFloat(texture2D(_sampler, texCoord) );

    float visibility = clamp(exp(_depthMultiplier * (occluder-receiver) ), 0.0, 1.0);

    return visibility;
}


vec4 blur9(sampler2D _sampler, vec2 _uv0, vec4 _uv1, vec4 _uv2, vec4 _uv3, vec4 _uv4)
{
#define _BLUR9_WEIGHT_0 1.0
#define _BLUR9_WEIGHT_1 0.9
#define _BLUR9_WEIGHT_2 0.55
#define _BLUR9_WEIGHT_3 0.18
#define _BLUR9_WEIGHT_4 0.1
#define _BLUR9_NORMALIZE (_BLUR9_WEIGHT_0+2.0*(_BLUR9_WEIGHT_1+_BLUR9_WEIGHT_2+_BLUR9_WEIGHT_3+_BLUR9_WEIGHT_4) )
#define BLUR9_WEIGHT(_x) (_BLUR9_WEIGHT_##_x/_BLUR9_NORMALIZE)

    float blur;
    blur  = unpackRgbaToFloat(texture2D(_sampler, _uv0)    * BLUR9_WEIGHT(0));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv1.xy) * BLUR9_WEIGHT(1));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv1.zw) * BLUR9_WEIGHT(1));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv2.xy) * BLUR9_WEIGHT(2));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv2.zw) * BLUR9_WEIGHT(2));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv3.xy) * BLUR9_WEIGHT(3));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv3.zw) * BLUR9_WEIGHT(3));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv4.xy) * BLUR9_WEIGHT(4));
    blur += unpackRgbaToFloat(texture2D(_sampler, _uv4.zw) * BLUR9_WEIGHT(4));
    return packFloatToRgba(blur);
}

vec4 blur9VSM(sampler2D _sampler, vec2 _uv0, vec4 _uv1, vec4 _uv2, vec4 _uv3, vec4 _uv4)
{
#define _BLUR9_WEIGHT_0 1.0
#define _BLUR9_WEIGHT_1 0.9
#define _BLUR9_WEIGHT_2 0.55
#define _BLUR9_WEIGHT_3 0.18
#define _BLUR9_WEIGHT_4 0.1
#define _BLUR9_NORMALIZE (_BLUR9_WEIGHT_0+2.0*(_BLUR9_WEIGHT_1+_BLUR9_WEIGHT_2+_BLUR9_WEIGHT_3+_BLUR9_WEIGHT_4) )
#define BLUR9_WEIGHT(_x) (_BLUR9_WEIGHT_##_x/_BLUR9_NORMALIZE)

    vec2 blur;
    vec4 val;
    val = texture2D(_sampler, _uv0) * BLUR9_WEIGHT(0);
    blur = vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv1.xy) * BLUR9_WEIGHT(1);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv1.zw) * BLUR9_WEIGHT(1);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv2.xy) * BLUR9_WEIGHT(2);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv2.zw) * BLUR9_WEIGHT(2);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv3.xy) * BLUR9_WEIGHT(3);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv3.zw) * BLUR9_WEIGHT(3);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv4.xy) * BLUR9_WEIGHT(4);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));
    val = texture2D(_sampler, _uv4.zw) * BLUR9_WEIGHT(4);
    blur += vec2(unpackHalfFloat(val.rg), unpackHalfFloat(val.ba));

    return vec4(packHalfFloat(blur.x), packHalfFloat(blur.y));
}


float findBlocker(sampler2D _sampler, vec4 _shadowCoord, vec2 _searchSize, float _bias)
{



#define BLOCKER_SEARCH_NUM_SAMPLES 16

    int blockerCount = 0;
    float avgBlockerDepth = 0.0;
    vec2 texCoord = _shadowCoord.xy / _shadowCoord.w;

    // Search around the shadow coordinate to find blockers
    for( int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i )
    {
        vec2 offset = samplePoisson(i) * _searchSize;
        float shadowMapDepth = unpackRgbaToFloat(texture2D(_sampler, texCoord + offset));
        if (shadowMapDepth < _shadowCoord.z - _bias)
        {
            avgBlockerDepth += shadowMapDepth;
            blockerCount++;
        }
    }


    // Calculate average blocker depth
    if (blockerCount > 0)
    {
        avgBlockerDepth /= float(blockerCount);
    }
    else
    {
        avgBlockerDepth = -1.0; // No blockers found
    }

    return avgBlockerDepth;
}



float PCSS(sampler2D _sampler, vec4 _shadowCoord, float _bias, vec4 _pcfParams, vec2 _texelSize)
{
    vec2 offset = _pcfParams.zw * _texelSize * _shadowCoord.w;

    // Step 1: Blocker Search
    float avgBlockerDepth = findBlocker(_sampler, _shadowCoord, offset, _bias);

    // If no blocker is found, return full visibility
    if (avgBlockerDepth == -1.0)
    {
        return 1.0;
    }

    vec4 _pcssParams = vec4(0.05, 2.0, 0.005, 0.1);  // Adjust as needed

    // Step 2: Penumbra Size Calculation
    float penumbraSize = (_shadowCoord.z - avgBlockerDepth) / avgBlockerDepth;
    penumbraSize *= _pcssParams.y; // Scale penumbra size for softer shadows

    // Step 3: Calculate filter radius in UV space
    float filterRadiusUV = clamp(penumbraSize * _pcssParams.x, _pcssParams.z, _pcssParams.w); // Scale and clamp the filter radius

    return PCFLodOffset(_sampler, _shadowCoord.z, vec2(filterRadiusUV, filterRadiusUV), _shadowCoord, _bias, _pcfParams, _texelSize);
}
