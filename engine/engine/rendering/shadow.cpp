#include "shadow.h"

#include <engine/assets/asset_manager.h>
#include <engine/defaults/defaults.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/engine.h>
#include <engine/events.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/rendering/renderer.h>

#include <graphics/index_buffer.h>
#include <graphics/render_pass.h>
#include <graphics/render_view.h>
#include <graphics/texture.h>
#include <graphics/vertex_buffer.h>

namespace ace
{

auto convert(light_type t) -> LightType::Enum
{
    static_assert(std::uint8_t(light_type::count) == std::uint8_t(LightType::Count), "Missing impl");

    switch(t)
    {
        case light_type::spot:
            return LightType::SpotLight;
        case light_type::point:
            return LightType::PointLight;
        default:
            return LightType::DirectionalLight;
    }
}

auto convert(sm_impl t) -> SmImpl::Enum
{
    static_assert(std::uint8_t(sm_impl::count) == std::uint8_t(SmImpl::Count), "Missing impl");

    switch(t)
    {
        case sm_impl::hard:
            return SmImpl::Hard;

        case sm_impl::pcf:
            return SmImpl::PCF;

        case sm_impl::esm:
            return SmImpl::ESM;

        case sm_impl::vsm:
            return SmImpl::VSM;
        default:
            return SmImpl::Count;
    }
}

auto convert(sm_depth t) -> DepthImpl::Enum
{
    static_assert(std::uint8_t(sm_depth::count) == std::uint8_t(DepthImpl::Count), "Missing impl");
    switch(t)
    {
        case sm_depth::invz:
            return DepthImpl::InvZ;

        case sm_depth::linear:
            return DepthImpl::Linear;

        default:
            return DepthImpl::Count;
    }
}

auto convert(sm_resolution t) -> float
{
    switch(t)
    {
        case sm_resolution::low:
            return 9;

        case sm_resolution::medium:
            return 10;

        case sm_resolution::high:
            return 11;

        case sm_resolution::very_high:
            return 12;

        default:
            return 10;
    }
}

void mtxBillboard(float* _result, const float* _view, const float* _pos, const float* _scale)
{
    _result[0] = _view[0] * _scale[0];
    _result[1] = _view[4] * _scale[0];
    _result[2] = _view[8] * _scale[0];
    _result[3] = 0.0f;
    _result[4] = _view[1] * _scale[1];
    _result[5] = _view[5] * _scale[1];
    _result[6] = _view[9] * _scale[1];
    _result[7] = 0.0f;
    _result[8] = _view[2] * _scale[2];
    _result[9] = _view[6] * _scale[2];
    _result[10] = _view[10] * _scale[2];
    _result[11] = 0.0f;
    _result[12] = _pos[0];
    _result[13] = _pos[1];
    _result[14] = _pos[2];
    _result[15] = 1.0f;
}

void mtxYawPitchRoll(float* _result, float _yaw, float _pitch, float _roll)
{
    float sroll = bx::sin(_roll);
    float croll = bx::cos(_roll);
    float spitch = bx::sin(_pitch);
    float cpitch = bx::cos(_pitch);
    float syaw = bx::sin(_yaw);
    float cyaw = bx::cos(_yaw);

    _result[0] = sroll * spitch * syaw + croll * cyaw;
    _result[1] = sroll * cpitch;
    _result[2] = sroll * spitch * cyaw - croll * syaw;
    _result[3] = 0.0f;
    _result[4] = croll * spitch * syaw - sroll * cyaw;
    _result[5] = croll * cpitch;
    _result[6] = croll * spitch * cyaw + sroll * syaw;
    _result[7] = 0.0f;
    _result[8] = cpitch * syaw;
    _result[9] = -spitch;
    _result[10] = cpitch * cyaw;
    _result[11] = 0.0f;
    _result[12] = 0.0f;
    _result[13] = 0.0f;
    _result[14] = 0.0f;
    _result[15] = 1.0f;
}

void screenSpaceQuad(bool _originBottomLeft = true, float _width = 1.0f, float _height = 1.0f)
{
    if(3 == bgfx::getAvailTransientVertexBuffer(3, PosColorTexCoord0Vertex::ms_layout))
    {
        bgfx::TransientVertexBuffer vb;
        bgfx::allocTransientVertexBuffer(&vb, 3, PosColorTexCoord0Vertex::ms_layout);
        PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

        const float zz = 0.0f;

        const float minx = -_width;
        const float maxx = _width;
        const float miny = 0.0f;
        const float maxy = _height * 2.0f;

        const float minu = -1.0f;
        const float maxu = 1.0f;

        float minv = 0.0f;
        float maxv = 2.0f;

        if(_originBottomLeft)
        {
            std::swap(minv, maxv);
            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_rgba = 0xffffffff;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_rgba = 0xffffffff;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_rgba = 0xffffffff;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;

        bgfx::setVertexBuffer(0, &vb);
    }
}

void worldSpaceFrustumCorners(float* _corners24f,
                              float _near,
                              float _far,
                              float _projWidth,
                              float _projHeight,
                              const float* _invViewMtx)
{
    // Define frustum corners in view space.
    const float nw = _near * _projWidth;
    const float nh = _near * _projHeight;
    const float fw = _far * _projWidth;
    const float fh = _far * _projHeight;

    const uint8_t numCorners = 8;
    const bx::Vec3 corners[numCorners] = {
        {-nw, nh, _near},
        {nw, nh, _near},
        {nw, -nh, _near},
        {-nw, -nh, _near},
        {-fw, fh, _far},
        {fw, fh, _far},
        {fw, -fh, _far},
        {-fw, -fh, _far},
    };

    // Convert them to world space.
    float(*out)[3] = (float(*)[3])_corners24f;
    for(uint8_t ii = 0; ii < numCorners; ++ii)
    {
        bx::store(&out[ii], bx::mul(corners[ii], _invViewMtx));
    }
}

// Function to compute frustum corners in world space for a given frustum slice
void worldSpaceFrustumCorners(float corners[8][3], const float* projMatrix, float nearSlice, float farSlice)
{
    // Define the frustum slice corners in clip space
    const bx::Vec3 clipSpaceCorners[8] = {
        {-1.0f, 1.0f, nearSlice},  // Near Top Left
        {1.0f, 1.0f, nearSlice},   // Near Top Right
        {1.0f, -1.0f, nearSlice},  // Near Bottom Right
        {-1.0f, -1.0f, nearSlice}, // Near Bottom Left
        {-1.0f, 1.0f, farSlice},   // Far Top Left
        {1.0f, 1.0f, farSlice},    // Far Top Right
        {1.0f, -1.0f, farSlice},   // Far Bottom Right
        {-1.0f, -1.0f, farSlice}   // Far Bottom Left
    };

    // Compute the inverse of the projection matrix
    float invProjMatrix[16];
    bx::mtxInverse(invProjMatrix, projMatrix);

    // Transform clip space corners to world space
    for(int i = 0; i < 8; ++i)
    {
        // Convert the clip space corner to a 4D vector with w = 1.0
        float clipSpaceCorner[4] = {clipSpaceCorners[i].x, clipSpaceCorners[i].y, clipSpaceCorners[i].z, 1.0f};

        // Transform the corner to world space
        float worldSpaceCorner[4];
        bx::vec4MulMtx(worldSpaceCorner, clipSpaceCorner, invProjMatrix);

        // Perform perspective divide to get the 3D coordinates
        float w = worldSpaceCorner[3];
        corners[i][0] = worldSpaceCorner[0] / w;
        corners[i][1] = worldSpaceCorner[1] / w;
        corners[i][2] = worldSpaceCorner[2] / w;
    }
}

/**
 * _splits = { near0, far0, near1, far1... nearN, farN }
 * N = _numSplits
 */
void splitFrustum(float* _splits, uint8_t _numSplits, float _near, float _far, float _splitWeight = 0.75f)
{
    const float l = _splitWeight;
    const float ratio = _far / _near;
    const int8_t numSlices = _numSplits * 2;
    const float numSlicesf = float(numSlices);

    // First slice.
    _splits[0] = _near;

    for(uint8_t nn = 2, ff = 1; nn < numSlices; nn += 2, ff += 2)
    {
        float si = float(int8_t(ff)) / numSlicesf;

        const float nearp = l * (_near * bx::pow(ratio, si)) + (1 - l) * (_near + (_far - _near) * si);
        _splits[nn] = nearp;          // near
        _splits[ff] = nearp * 1.005f; // far from previous split
    }

    // Last slice.
    _splits[numSlices - 1] = _far;
}

bgfx::VertexLayout PosColorTexCoord0Vertex::ms_layout;

void shadow::init(rtti::context& ctx)
{
    if(bgfx::isValid(s_texColor))
    {
        return;
    }
    // Uniforms.
    s_uniforms.init();
    s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    s_shadowMap[0] = bgfx::createUniform("s_shadowMap0", bgfx::UniformType::Sampler);
    s_shadowMap[1] = bgfx::createUniform("s_shadowMap1", bgfx::UniformType::Sampler);
    s_shadowMap[2] = bgfx::createUniform("s_shadowMap2", bgfx::UniformType::Sampler);
    s_shadowMap[3] = bgfx::createUniform("s_shadowMap3", bgfx::UniformType::Sampler);

    // Programs.
    s_programs.init(ctx);

    // Vertex declarations.
    bgfx::VertexLayout PosNormalTexcoordLayout;
    PosNormalTexcoordLayout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    m_posLayout.begin();
    m_posLayout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
    m_posLayout.end();

    PosColorTexCoord0Vertex::init();

    // Lights.
    // clang-format off
    m_pointLight =
        {
            { { 0.0f, 0.0f, 0.0f, 1.0f   } }, //position
            { { 0.0f,-0.4f,-0.6f, 0.0f   } }, //spotdirection, spotexponent
        };

    m_directionalLight =
        {
            { { 0.5f,-1.0f, 0.1f, 0.0f  } }, //position
            { { 0.0f, 0.0f, 0.0f, 1.0f  } }, //spotdirection, spotexponent
        };

    // clang-format on

    // Setup uniforms.
    m_color[0] = m_color[1] = m_color[2] = m_color[3] = 1.0f;
    s_uniforms.setPtrs(&m_pointLight,
                       m_color,
                       m_lightMtx,
                       &m_shadowMapMtx[ShadowMapRenderTargets::First][0],
                       &m_shadowMapMtx[ShadowMapRenderTargets::Second][0],
                       &m_shadowMapMtx[ShadowMapRenderTargets::Third][0],
                       &m_shadowMapMtx[ShadowMapRenderTargets::Fourth][0]);
    s_uniforms.submitConstUniforms();

    // clang-format off
    // Settings.
    ShadowMapSettings smSettings[LightType::Count][DepthImpl::Count][SmImpl::Count] =
    {
        { //LightType::Spot

            { //DepthImpl::InvZ

                { //SmImpl::Hard
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0035f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.0012f, 0.0f, 0.05f, 0.00001f   // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 500.0f, 1.0f, 1000.0f, 1.0f      // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::Hard].get() //m_progDraw
                },
                { //SmImpl::PCF
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 1.0f, 1.0f, 99.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.007f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 500.0f, 1.0f, 1000.0f, 1.0f      // m_customParam1
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::PCF].get() //m_progDraw
                },
                { //SmImpl::VSM
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 8.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.045f, 0.0f, 0.1f, 0.00001f     // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.02f, 0.0f, 0.04f, 0.00001f     // m_customParam0
                    , 450.0f, 1.0f, 1000.0f, 1.0f      // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::VSM].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::VSM].get() //m_progDraw
                },
                { //SmImpl::ESM
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 3.0f, 1.0f, 10.0f, 0.01f         // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.02f, 0.0f, 0.3f, 0.00001f      // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 9000.0f, 1.0f, 15000.0f, 1.0f    // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::ESM].get() //m_progDraw
                }

            },
            { //DepthImpl::Linear

                { //SmImpl::Hard
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0025f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.0012f, 0.0f, 0.05f, 0.00001f   // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 500.0f, 1.0f, 1000.0f, 1.0f      // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::Hard].get() //m_progDraw
                },
                { //SmImpl::PCF
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 99.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0025f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.001f, 0.0f, 0.05f,  0.00001f   // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 2000.0f, 1.0f, 2000.0f, 1.0f     // m_customParam1
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::PCF].get() //m_progDraw
                },
                { //SmImpl::VSM
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.006f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.02f, 0.0f, 0.1f, 0.00001f      // m_customParam0
                    , 300.0f, 1.0f, 1500.0f, 1.0f      // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::VSM].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::VSM].get() //m_progDraw
                },
                { //SmImpl::ESM
                    10.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 0.01f         // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0055f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 2500.0f, 1.0f, 5000.0f, 1.0f     // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::ESM].get() //m_progDraw
                }

            }

        },
        { //LightType::Point

            { //DepthImpl::InvZ

                { //SmImpl::Hard
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.006f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 50.0f, 1.0f, 300.0f, 1.0f        // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_xOffset
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::Hard].get() //m_progDraw
                },
                { //SmImpl::PCF
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 1.0f, 1.0f, 99.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.004f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 50.0f, 1.0f, 300.0f, 1.0f        // m_customParam1
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.001f         // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.001f         // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::PCF].get() //m_progDraw
                },
                { //SmImpl::VSM
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 8.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.055f, 0.0f, 0.1f, 0.00001f     // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.02f, 0.0f, 0.04f, 0.00001f     // m_customParam0
                    , 450.0f, 1.0f, 900.0f, 1.0f       // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_xOffset
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::VSM].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::VSM].get() //m_progDraw
                },
                { //SmImpl::ESM
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 10.0f, 1.0f, 20.0f, 1.0f         // m_depthValuePow
                    , 3.0f, 1.0f, 10.0f, 0.01f         // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.035f, 0.0f, 0.1f, 0.00001f     // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 9000.0f, 1.0f, 15000.0f, 1.0f    // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_xOffset
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::ESM].get() //m_progDraw
                }

            },
            { //DepthImpl::Linear

                { //SmImpl::Hard
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.003f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 120.0f, 1.0f, 300.0f, 1.0f       // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_xOffset
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::Hard].get() //m_progDraw
                },
                { //SmImpl::PCF
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 99.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0035f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 120.0f, 1.0f, 300.0f, 1.0f       // m_customParam1
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.001f         // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.001f         // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::PCF].get() //m_progDraw
                },
                { //SmImpl::VSM
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.006f, 0.0f, 0.1f, 0.00001f     // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.02f, 0.0f, 0.1f, 0.00001f      // m_customParam0
                    , 400.0f, 1.0f, 900.0f, 1.0f       // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_xOffset
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::VSM].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::VSM].get() //m_progDraw
                },
                { //SmImpl::ESM
                    12.0f, 9.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 0.01f         // m_near
                    , 250.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.007f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.05f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 8000.0f, 1.0f, 15000.0f, 1.0f    // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_xOffset
                    , 0.25f, 0.0f, 2.0f, 0.001f        // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::ESM].get() //m_progDraw
                }

            }

        },
        { //LightType::Directional

            { //DepthImpl::InvZ

                { //SmImpl::Hard
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0012f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 200.0f, 1.0f, 400.0f, 1.0f       // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_xOffset
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::Hard].get() //m_progDraw
                },
                { //SmImpl::PCF
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 99.0f, 1.0f          // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0012f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 200.0f, 1.0f, 400.0f, 1.0f       // m_customParam1
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::PCF].get() //m_progDraw
                },
                { //SmImpl::VSM
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.004f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.02f, 0.0f, 0.04f, 0.00001f     // m_customParam0
                    , 2500.0f, 1.0f, 5000.0f, 1.0f     // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_xOffset
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::VSM].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::VSM].get() //m_progDraw
                },
                { //SmImpl::ESM
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 0.01f         // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.004f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 9500.0f, 1.0f, 15000.0f, 1.0f    // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_xOffset
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::ESM].get() //m_progDraw
                }

            },
            { //DepthImpl::Linear

                { //SmImpl::Hard
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0012f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 500.0f, 1.0f, 1000.0f, 1.0f      // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_xOffset
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::Hard].get() //m_progDraw
                },
                { //SmImpl::PCF
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 99.0f, 1.0f          // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.0012f, 0.0f, 0.01f, 0.00001f   // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 200.0f, 1.0f, 400.0f, 1.0f       // m_customParam1
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 8.0f, 1.0f           // m_yNum
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_xOffset
                    , 1.0f, 0.0f, 3.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::PCF].get() //m_progDraw
                },
                { //SmImpl::VSM
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 1.0f          // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.004f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.02f, 0.0f, 0.04f, 0.00001f     // m_customParam0
                    , 2500.0f, 1.0f, 5000.0f, 1.0f     // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_xOffset
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::VSM].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::VSM].get() //m_progDraw
                },
                { //SmImpl::ESM
                    11.0f, 7.0f, 12.0f, 1.0f         // m_sizePwrTwo
                    , 1.0f, 1.0f, 20.0f, 1.0f          // m_depthValuePow
                    , 1.0f, 1.0f, 10.0f, 0.01f         // m_near
                    , 550.0f, 100.0f, 2000.0f, 50.0f   // m_far
                    , 0.004f, 0.0f, 0.01f, 0.00001f    // m_bias
                    , 0.001f, 0.0f, 0.04f, 0.00001f    // m_normalOffset
                    , 0.7f, 0.0f, 1.0f, 0.01f          // m_customParam0
                    , 9500.0f, 1.0f, 15000.0f, 1.0f    // m_customParam1
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_xNum
                    , 2.0f, 0.0f, 4.0f, 1.0f           // m_yNum
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_xOffset
                    , 0.2f, 0.0f, 1.0f, 0.01f          // m_yOffset
                    , true                             // m_doBlur
                    , s_programs.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , s_programs.m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::ESM].get() //m_progDraw
                }

            }
        }
    };
    // clang-format on
    bx::memCopy(m_smSettings, smSettings, sizeof(smSettings));

    m_settings.m_lightType = LightType::SpotLight;
    m_settings.m_depthImpl = DepthImpl::InvZ;
    m_settings.m_smImpl = SmImpl::Hard;
    m_settings.m_spotOuterAngle = 45.0f;
    m_settings.m_spotInnerAngle = 30.0f;
    m_settings.m_fovXAdjust = 0.0f;
    m_settings.m_fovYAdjust = 0.0f;
    m_settings.m_coverageSpotL = 90.0f;
    m_settings.m_splitDistribution = 0.6f;
    m_settings.m_numSplits = 4;
    m_settings.m_updateLights = true;
    m_settings.m_updateScene = true;
    m_settings.m_drawDepthBuffer = false;
    m_settings.m_showSmCoverage = false;
    m_settings.m_stencilPack = true;
    m_settings.m_stabilize = true;

    ShadowMapSettings* currentSmSettings =
        &m_smSettings[m_settings.m_lightType][m_settings.m_depthImpl][m_settings.m_smImpl];

    // Render targets.
    uint16_t shadowMapSize = 1 << uint32_t(currentSmSettings->m_sizePwrTwo);
    m_currentShadowMapSize = shadowMapSize;
    float currentShadowMapSizef = float(int16_t(m_currentShadowMapSize));
    s_uniforms.m_shadowMapTexelSize = 1.0f / currentShadowMapSizef;
    for(uint8_t ii = 0; ii < ShadowMapRenderTargets::Count; ++ii)
    {
        bgfx::TextureHandle fbtextures[] = {
            bgfx::createTexture2D(m_currentShadowMapSize,
                                  m_currentShadowMapSize,
                                  false,
                                  1,
                                  bgfx::TextureFormat::BGRA8,
                                  BGFX_TEXTURE_RT),
            bgfx::createTexture2D(m_currentShadowMapSize,
                                  m_currentShadowMapSize,
                                  false,
                                  1,
                                  bgfx::TextureFormat::D32F,
                                  BGFX_TEXTURE_RT),
        };
        s_rtShadowMap[ii] = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
    }
    s_rtBlur = bgfx::createFrameBuffer(m_currentShadowMapSize, m_currentShadowMapSize, bgfx::TextureFormat::BGRA8);
}

auto shadow::get_depth_type() const -> PackDepth::Enum
{
    PackDepth::Enum depthType = (SmImpl::VSM == m_settings.m_smImpl) ? PackDepth::VSM : PackDepth::RGBA;
    return depthType;
}

auto shadow::get_rt_texture(uint8_t split) const -> bgfx::TextureHandle
{
    return bgfx::getTexture(s_rtShadowMap[split]);
}

auto shadow::get_depth_render_program(PackDepth::Enum depth) const -> bgfx::ProgramHandle
{
    return s_programs.m_drawDepth[depth];
}

void shadow::submit_uniforms()
{
    if(!bgfx::isValid(s_texColor))
    {
        return;
    }
    s_uniforms.submitConstUniforms();
    s_uniforms.submitPerFrameUniforms();
    s_uniforms.submitPerDrawUniforms();

    for(uint8_t ii = 0; ii < ShadowMapRenderTargets::Count; ++ii)
    {
        bgfx::setTexture(7 + ii, s_shadowMap[ii], bgfx::getTexture(s_rtShadowMap[ii]));
    }
}

auto shadow::get_color_apply_program(const light& l) -> gpu_program*
{
    init(engine::context());

    auto lightType = convert(l.type);
    auto smImpl = convert(l.shadow_params.type);
    auto depthImpl = convert(l.shadow_params.depth);

    return m_smSettings[lightType][depthImpl][smImpl].m_progDraw;
}

void shadow::generate_shadowmaps(const light& l, const math::transform& ltrans, const shadow_map_models_t& models)
{
    init(engine::context());

    {
        const auto& pos = ltrans.get_position();
        const auto& dir = ltrans.z_unit_axis();
        m_pointLight.m_position.m_x = pos.x;
        m_pointLight.m_position.m_y = pos.y;
        m_pointLight.m_position.m_z = pos.z;

        m_pointLight.m_spotDirectionInner.m_x = dir.x;
        m_pointLight.m_spotDirectionInner.m_y = dir.y;
        m_pointLight.m_spotDirectionInner.m_z = dir.z;

        m_directionalLight.m_position.m_x = dir.x;
        m_directionalLight.m_position.m_y = dir.y;
        m_directionalLight.m_position.m_z = dir.z;

        m_settings.m_lightType = convert(l.type);
        m_settings.m_smImpl = convert(l.shadow_params.type);
        m_settings.m_depthImpl = convert(l.shadow_params.depth);

        m_settings.m_showSmCoverage = l.shadow_params.show_coverage;

        switch(l.type)
        {
            case light_type::spot:
                m_settings.m_spotOuterAngle = l.spot_data.get_outer_angle();
                m_settings.m_spotInnerAngle = l.spot_data.get_inner_angle();
                m_settings.m_coverageSpotL = m_settings.m_spotOuterAngle;
                break;
            case light_type::point:
                m_settings.m_stencilPack = l.point_data.shadow_params.stencil_pack;
                m_settings.m_fovXAdjust = l.point_data.shadow_params.fov_x_adjust;
                m_settings.m_fovYAdjust = l.point_data.shadow_params.fov_y_adjust;

                break;
            default:

                m_settings.m_splitDistribution = l.directional_data.shadow_params.split_distribution;
                m_settings.m_numSplits = l.directional_data.shadow_params.num_splits;
                m_settings.m_stabilize = l.directional_data.shadow_params.stabilize;

                break;
        }
    }

    gfx::render_pass RENDERVIEW_SHADOWMAP_0_PASS("RENDERVIEW_SHADOWMAP_0_ID");
    gfx::render_pass RENDERVIEW_SHADOWMAP_1_PASS("RENDERVIEW_SHADOWMAP_1_ID");
    gfx::render_pass RENDERVIEW_SHADOWMAP_2_PASS("RENDERVIEW_SHADOWMAP_2_ID");
    gfx::render_pass RENDERVIEW_SHADOWMAP_3_PASS("RENDERVIEW_SHADOWMAP_3_ID");
    gfx::render_pass RENDERVIEW_SHADOWMAP_4_PASS("RENDERVIEW_SHADOWMAP_4_ID");
    gfx::render_pass RENDERVIEW_VBLUR_0_PASS("RENDERVIEW_VBLUR_0_ID");
    gfx::render_pass RENDERVIEW_HBLUR_0_PASS("RENDERVIEW_HBLUR_0_ID");
    gfx::render_pass RENDERVIEW_VBLUR_1_PASS("RENDERVIEW_VBLUR_1_ID");
    gfx::render_pass RENDERVIEW_HBLUR_1_PASS("RENDERVIEW_HBLUR_1_ID");
    gfx::render_pass RENDERVIEW_VBLUR_2_PASS("RENDERVIEW_VBLUR_2_ID");
    gfx::render_pass RENDERVIEW_HBLUR_2_PASS("RENDERVIEW_HBLUR_2_ID");
    gfx::render_pass RENDERVIEW_VBLUR_3_PASS("RENDERVIEW_VBLUR_3_ID");
    gfx::render_pass RENDERVIEW_HBLUR_3_PASS("RENDERVIEW_HBLUR_3_ID");

    auto RENDERVIEW_SHADOWMAP_0_ID = RENDERVIEW_SHADOWMAP_0_PASS.id;
    auto RENDERVIEW_SHADOWMAP_1_ID = RENDERVIEW_SHADOWMAP_1_PASS.id;
    auto RENDERVIEW_SHADOWMAP_2_ID = RENDERVIEW_SHADOWMAP_2_PASS.id;
    auto RENDERVIEW_SHADOWMAP_3_ID = RENDERVIEW_SHADOWMAP_3_PASS.id;
    auto RENDERVIEW_SHADOWMAP_4_ID = RENDERVIEW_SHADOWMAP_4_PASS.id;
    auto RENDERVIEW_VBLUR_0_ID = RENDERVIEW_VBLUR_0_PASS.id;
    auto RENDERVIEW_HBLUR_0_ID = RENDERVIEW_HBLUR_0_PASS.id;
    auto RENDERVIEW_VBLUR_1_ID = RENDERVIEW_VBLUR_1_PASS.id;
    auto RENDERVIEW_HBLUR_1_ID = RENDERVIEW_HBLUR_1_PASS.id;
    auto RENDERVIEW_VBLUR_2_ID = RENDERVIEW_VBLUR_2_PASS.id;
    auto RENDERVIEW_HBLUR_2_ID = RENDERVIEW_HBLUR_2_PASS.id;
    auto RENDERVIEW_VBLUR_3_ID = RENDERVIEW_VBLUR_3_PASS.id;
    auto RENDERVIEW_HBLUR_3_ID = RENDERVIEW_HBLUR_3_PASS.id;

    bool bLtChanged = false;
    const bgfx::Caps* caps = bgfx::getCaps();

    bool s_originBottomLeft = caps->originBottomLeft;

    float currentShadowMapSizef = float(int16_t(m_currentShadowMapSize));
    float shadowMapTexelSize = 1.0f / currentShadowMapSizef;
    s_uniforms.m_shadowMapTexelSize = shadowMapTexelSize;

    s_uniforms.submitConstUniforms();

    ShadowMapSettings* currentSmSettings =
        &m_smSettings[m_settings.m_lightType][m_settings.m_depthImpl][m_settings.m_smImpl];

    currentSmSettings->m_sizePwrTwo = convert(l.shadow_params.resolution);
    currentSmSettings->m_near = l.shadow_params.near_plane;
    currentSmSettings->m_bias = l.shadow_params.bias;
    currentSmSettings->m_normalOffset = l.shadow_params.normal_bias;

    switch(l.type)
    {
        case light_type::spot:
            currentSmSettings->m_far = l.spot_data.range;
            break;
        case light_type::point:
            currentSmSettings->m_far = l.point_data.range;
            break;
        default:
            currentSmSettings->m_far = l.shadow_params.far_plane;
            break;
    }

    // Update uniforms.
    s_uniforms.m_shadowMapBias = currentSmSettings->m_bias;
    s_uniforms.m_shadowMapOffset = currentSmSettings->m_normalOffset;
    s_uniforms.m_shadowMapParam0 = currentSmSettings->m_customParam0;
    s_uniforms.m_shadowMapParam1 = currentSmSettings->m_customParam1;
    s_uniforms.m_depthValuePow = currentSmSettings->m_depthValuePow;
    s_uniforms.m_XNum = currentSmSettings->m_xNum;
    s_uniforms.m_YNum = currentSmSettings->m_yNum;
    s_uniforms.m_XOffset = currentSmSettings->m_xOffset;
    s_uniforms.m_YOffset = currentSmSettings->m_yOffset;
    s_uniforms.m_showSmCoverage = float(m_settings.m_showSmCoverage);
    s_uniforms.m_lightPtr =
        (LightType::DirectionalLight == m_settings.m_lightType) ? &m_directionalLight : &m_pointLight;

    if(LightType::SpotLight == m_settings.m_lightType)
    {
        m_pointLight.m_spotDirectionInner.m_inner = m_settings.m_spotInnerAngle;
    }

    s_uniforms.submitPerFrameUniforms();

    // Compute transform matrices.
    const uint8_t shadowMapPasses = ShadowMapRenderTargets::Count;
    float lightView[shadowMapPasses][16];
    float lightProj[shadowMapPasses][16];
    float mtxYpr[TetrahedronFaces::Count][16];

    float screenProj[16];
    float screenView[16];
    bx::mtxIdentity(screenView);

    bx::mtxOrtho(screenProj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, caps->homogeneousDepth);

    // Update render target size.
    uint16_t shadowMapSize = 1 << uint32_t(currentSmSettings->m_sizePwrTwo);
    if(bLtChanged || m_currentShadowMapSize != shadowMapSize)
    {
        m_currentShadowMapSize = shadowMapSize;
        s_uniforms.m_shadowMapTexelSize = 1.0f / currentShadowMapSizef;

        {
            bgfx::destroy(s_rtShadowMap[0]);

            bgfx::TextureHandle fbtextures[] = {
                bgfx::createTexture2D(m_currentShadowMapSize,
                                      m_currentShadowMapSize,
                                      false,
                                      1,
                                      bgfx::TextureFormat::BGRA8,
                                      BGFX_TEXTURE_RT),
                bgfx::createTexture2D(m_currentShadowMapSize,
                                      m_currentShadowMapSize,
                                      false,
                                      1,
                                      bgfx::TextureFormat::D24S8,
                                      BGFX_TEXTURE_RT),
            };
            s_rtShadowMap[0] = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
        }

        if(LightType::DirectionalLight == m_settings.m_lightType)
        {
            for(uint8_t ii = 1; ii < ShadowMapRenderTargets::Count; ++ii)
            {
                {
                    bgfx::destroy(s_rtShadowMap[ii]);

                    bgfx::TextureHandle fbtextures[] = {
                        bgfx::createTexture2D(m_currentShadowMapSize,
                                              m_currentShadowMapSize,
                                              false,
                                              1,
                                              bgfx::TextureFormat::BGRA8,
                                              BGFX_TEXTURE_RT),
                        bgfx::createTexture2D(m_currentShadowMapSize,
                                              m_currentShadowMapSize,
                                              false,
                                              1,
                                              bgfx::TextureFormat::D24S8,
                                              BGFX_TEXTURE_RT),
                    };
                    s_rtShadowMap[ii] = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
                }
            }
        }

        bgfx::destroy(s_rtBlur);
        s_rtBlur = bgfx::createFrameBuffer(m_currentShadowMapSize, m_currentShadowMapSize, bgfx::TextureFormat::BGRA8);
    }

    if(LightType::SpotLight == m_settings.m_lightType)
    {
        const float fovy = m_settings.m_coverageSpotL;
        const float aspect = 1.0f;
        bx::mtxProj(lightProj[ProjType::Horizontal],
                    fovy,
                    aspect,
                    currentSmSettings->m_near,
                    currentSmSettings->m_far,
                    false);

        // For linear depth, prevent depth division by variable w-component in shaders and divide here by far plane
        if(DepthImpl::Linear == m_settings.m_depthImpl)
        {
            lightProj[ProjType::Horizontal][10] /= currentSmSettings->m_far;
            lightProj[ProjType::Horizontal][14] /= currentSmSettings->m_far;
        }

        const bx::Vec3 at = bx::add(bx::load<bx::Vec3>(m_pointLight.m_position.m_v),
                                    bx::load<bx::Vec3>(m_pointLight.m_spotDirectionInner.m_v));
        bx::mtxLookAt(lightView[TetrahedronFaces::Green], bx::load<bx::Vec3>(m_pointLight.m_position.m_v), at);
    }
    else if(LightType::PointLight == m_settings.m_lightType)
    {
        float ypr[TetrahedronFaces::Count][3] = {
            {bx::toRad(0.0f), bx::toRad(27.36780516f), bx::toRad(0.0f)},
            {bx::toRad(180.0f), bx::toRad(27.36780516f), bx::toRad(0.0f)},
            {bx::toRad(-90.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f)},
            {bx::toRad(90.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f)},
        };

        if(m_settings.m_stencilPack)
        {
            const float fovx = 143.98570868f + 3.51f + m_settings.m_fovXAdjust;
            const float fovy = 125.26438968f + 9.85f + m_settings.m_fovYAdjust;
            const float aspect = bx::tan(bx::toRad(fovx * 0.5f)) / bx::tan(bx::toRad(fovy * 0.5f));

            bx::mtxProj(lightProj[ProjType::Vertical],
                        fovx,
                        aspect,
                        currentSmSettings->m_near,
                        currentSmSettings->m_far,
                        false);

            // For linear depth, prevent depth division by variable w-component in shaders and divide here by far plane
            if(DepthImpl::Linear == m_settings.m_depthImpl)
            {
                lightProj[ProjType::Vertical][10] /= currentSmSettings->m_far;
                lightProj[ProjType::Vertical][14] /= currentSmSettings->m_far;
            }

            ypr[TetrahedronFaces::Green][2] = bx::toRad(180.0f);
            ypr[TetrahedronFaces::Yellow][2] = bx::toRad(0.0f);
            ypr[TetrahedronFaces::Blue][2] = bx::toRad(90.0f);
            ypr[TetrahedronFaces::Red][2] = bx::toRad(-90.0f);
        }

        const float fovx = 143.98570868f + 7.8f + m_settings.m_fovXAdjust;
        const float fovy = 125.26438968f + 3.0f + m_settings.m_fovYAdjust;
        const float aspect = bx::tan(bx::toRad(fovx * 0.5f)) / bx::tan(bx::toRad(fovy * 0.5f));

        bx::mtxProj(lightProj[ProjType::Horizontal],
                    fovy,
                    aspect,
                    currentSmSettings->m_near,
                    currentSmSettings->m_far,
                    caps->homogeneousDepth);

        // For linear depth, prevent depth division by variable w component in shaders and divide here by far plane
        if(DepthImpl::Linear == m_settings.m_depthImpl)
        {
            lightProj[ProjType::Horizontal][10] /= currentSmSettings->m_far;
            lightProj[ProjType::Horizontal][14] /= currentSmSettings->m_far;
        }

        for(uint8_t ii = 0; ii < TetrahedronFaces::Count; ++ii)
        {
            float mtxTmp[16];
            mtxYawPitchRoll(mtxTmp, ypr[ii][0], ypr[ii][1], ypr[ii][2]);

            float tmp[3] = {
                -bx::dot(bx::load<bx::Vec3>(m_pointLight.m_position.m_v), bx::load<bx::Vec3>(&mtxTmp[0])),
                -bx::dot(bx::load<bx::Vec3>(m_pointLight.m_position.m_v), bx::load<bx::Vec3>(&mtxTmp[4])),
                -bx::dot(bx::load<bx::Vec3>(m_pointLight.m_position.m_v), bx::load<bx::Vec3>(&mtxTmp[8])),
            };

            bx::mtxTranspose(mtxYpr[ii], mtxTmp);

            bx::memCopy(lightView[ii], mtxYpr[ii], 12 * sizeof(float));
            lightView[ii][12] = tmp[0];
            lightView[ii][13] = tmp[1];
            lightView[ii][14] = tmp[2];
            lightView[ii][15] = 1.0f;
        }
    }
    else // LightType::DirectionalLight == m_settings.m_lightType
    {
        // Setup light view matrix to look at the origin.
        const bx::Vec3 eye = {
            -m_directionalLight.m_position.m_x,
            -m_directionalLight.m_position.m_y,
            -m_directionalLight.m_position.m_z,
        };
        const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
        // const bx::Vec3 at = bx::mul(eye, 100);
        bx::mtxLookAt(lightView[0], eye, at);

        const uint8_t maxNumSplits = 4;
        BX_ASSERT(maxNumSplits >= m_settings.m_numSplits, "Error! Max num splits.");

        // Define a fixed scene bounding box (min and max corners in world space)
        math::bbox scene_bounds{{-5.0f, -5.0f, -5.0f}, {5.0f, 5.0f, 5.0f}};
        for(const auto& e : models)
        {
            auto bounds = defaults::calc_bounds(e);

            scene_bounds.add_point(bounds.min);
            scene_bounds.add_point(bounds.max);
        }

        float mtxProj[16];
        bx::mtxOrtho(mtxProj,
                     1.0f,
                     -1.0f,
                     1.0f,
                     -1.0f,
                     -currentSmSettings->m_far,
                     currentSmSettings->m_far,
                     0.0f,
                     caps->homogeneousDepth);

        // Split distances
        float splitSlices[maxNumSplits * 2];
        splitFrustum(splitSlices,
                     uint8_t(m_settings.m_numSplits),
                     currentSmSettings->m_near,
                     currentSmSettings->m_far,
                     m_settings.m_splitDistribution);

        for(uint8_t ii = 0; ii < m_settings.m_numSplits; ++ii)
        {
            // Transform scene bounding box corners to light space
            bx::Vec3 corners[8] = {{scene_bounds.min.x, scene_bounds.min.y, scene_bounds.min.z},
                                   {scene_bounds.max.x, scene_bounds.min.y, scene_bounds.min.z},
                                   {scene_bounds.max.x, scene_bounds.max.y, scene_bounds.min.z},
                                   {scene_bounds.min.x, scene_bounds.max.y, scene_bounds.min.z},
                                   {scene_bounds.min.x, scene_bounds.min.y, scene_bounds.max.z},
                                   {scene_bounds.max.x, scene_bounds.min.y, scene_bounds.max.z},
                                   {scene_bounds.max.x, scene_bounds.max.y, scene_bounds.max.z},
                                   {scene_bounds.min.x, scene_bounds.max.y, scene_bounds.max.z}};

            bx::Vec3 min = {1e6f, 1e6f, 1e6f};
            bx::Vec3 max = {-1e6f, -1e6f, -1e6f};

            for(uint8_t jj = 0; jj < 8; ++jj)
            {
                // Transform to light space
                bx::Vec3 lightSpaceCorner = bx::mul(corners[jj], lightView[0]);

                // Update bounding box in light space
                min = bx::min(min, lightSpaceCorner);
                max = bx::max(max, lightSpaceCorner);
            }

            const bx::Vec3 minproj = bx::mulH(min, mtxProj);
            const bx::Vec3 maxproj = bx::mulH(max, mtxProj);

            float scalex = 2.0f / (maxproj.x - minproj.x);
            float scaley = 2.0f / (maxproj.y - minproj.y);

            if(m_settings.m_stabilize)
            {
                const float quantizer = 64.0f;
                scalex = quantizer / bx::ceil(quantizer / scalex);
                scaley = quantizer / bx::ceil(quantizer / scaley);
            }

            float offsetx = 0.5f * (maxproj.x + minproj.x) * scalex;
            float offsety = 0.5f * (maxproj.y + minproj.y) * scaley;

            if(m_settings.m_stabilize)
            {
                const float halfSize = currentShadowMapSizef * 0.5f;
                offsetx = bx::ceil(offsetx * halfSize) / halfSize;
                offsety = bx::ceil(offsety * halfSize) / halfSize;
            }

            float mtxCrop[16];
            bx::mtxIdentity(mtxCrop);
            mtxCrop[0] = scalex;
            mtxCrop[5] = scaley;
            mtxCrop[12] = offsetx;
            mtxCrop[13] = offsety;

            bx::mtxMul(lightProj[ii], mtxCrop, mtxProj);
        }
    }

    if(LightType::SpotLight == m_settings.m_lightType)
    {
        /**
         * RENDERVIEW_SHADOWMAP_0_ID - Clear shadow map. (used as convenience, otherwise render_pass_1 could be cleared)
         * RENDERVIEW_SHADOWMAP_1_ID - Craft shadow map.
         * RENDERVIEW_VBLUR_0_ID - Vertical blur.
         * RENDERVIEW_HBLUR_0_ID - Horizontal blur.
         * RENDERVIEW_DRAWSCENE_0_ID - Draw scene.
         * RENDERVIEW_DRAWSCENE_1_ID - Draw floor bottom.
         * RENDERVIEW_DRAWDEPTH_0_ID - Draw depth buffer.
         */

        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_VBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_HBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_1_ID, lightView[0], lightProj[ProjType::Horizontal]);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_0_ID, screenView, screenProj);

        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_0_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_1_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_0_ID, s_rtBlur);
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_0_ID, s_rtShadowMap[0]);
    }
    else if(LightType::PointLight == m_settings.m_lightType)
    {
        /**
         * RENDERVIEW_SHADOWMAP_0_ID - Clear entire shadow map.
         * RENDERVIEW_SHADOWMAP_1_ID - Craft green tetrahedron shadow face.
         * RENDERVIEW_SHADOWMAP_2_ID - Craft yellow tetrahedron shadow face.
         * RENDERVIEW_SHADOWMAP_3_ID - Craft blue tetrahedron shadow face.
         * RENDERVIEW_SHADOWMAP_4_ID - Craft red tetrahedron shadow face.
         * RENDERVIEW_VBLUR_0_ID - Vertical blur.
         * RENDERVIEW_HBLUR_0_ID - Horizontal blur.
         * RENDERVIEW_DRAWSCENE_0_ID - Draw scene.
         * RENDERVIEW_DRAWSCENE_1_ID - Draw floor bottom.
         * RENDERVIEW_DRAWDEPTH_0_ID - Draw depth buffer.
         */

        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        if(m_settings.m_stencilPack)
        {
            const uint16_t f = m_currentShadowMapSize;     // full size
            const uint16_t h = m_currentShadowMapSize / 2; // half size
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, f, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_2_ID, 0, h, f, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_3_ID, 0, 0, h, f);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_4_ID, h, 0, h, f);
        }
        else
        {
            const uint16_t h = m_currentShadowMapSize / 2; // half size
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, h, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_2_ID, h, 0, h, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_3_ID, 0, h, h, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_4_ID, h, h, h, h);
        }
        bgfx::setViewRect(RENDERVIEW_VBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_HBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_1_ID,
                               lightView[TetrahedronFaces::Green],
                               lightProj[ProjType::Horizontal]);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_2_ID,
                               lightView[TetrahedronFaces::Yellow],
                               lightProj[ProjType::Horizontal]);
        if(m_settings.m_stencilPack)
        {
            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_3_ID,
                                   lightView[TetrahedronFaces::Blue],
                                   lightProj[ProjType::Vertical]);
            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_4_ID,
                                   lightView[TetrahedronFaces::Red],
                                   lightProj[ProjType::Vertical]);
        }
        else
        {
            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_3_ID,
                                   lightView[TetrahedronFaces::Blue],
                                   lightProj[ProjType::Horizontal]);
            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_4_ID,
                                   lightView[TetrahedronFaces::Red],
                                   lightProj[ProjType::Horizontal]);
        }
        bgfx::setViewTransform(RENDERVIEW_VBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_0_ID, screenView, screenProj);

        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_0_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_1_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_2_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_3_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_4_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_0_ID, s_rtBlur);
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_0_ID, s_rtShadowMap[0]);
    }
    else // LightType::DirectionalLight == settings.m_lightType
    {
        /**
         * RENDERVIEW_SHADOWMAP_1_ID - Craft shadow map for first  split.
         * RENDERVIEW_SHADOWMAP_2_ID - Craft shadow map for second split.
         * RENDERVIEW_SHADOWMAP_3_ID - Craft shadow map for third  split.
         * RENDERVIEW_SHADOWMAP_4_ID - Craft shadow map for fourth split.
         * RENDERVIEW_VBLUR_0_ID - Vertical   blur for first  split.
         * RENDERVIEW_HBLUR_0_ID - Horizontal blur for first  split.
         * RENDERVIEW_VBLUR_1_ID - Vertical   blur for second split.
         * RENDERVIEW_HBLUR_1_ID - Horizontal blur for second split.
         * RENDERVIEW_VBLUR_2_ID - Vertical   blur for third  split.
         * RENDERVIEW_HBLUR_2_ID - Horizontal blur for third  split.
         * RENDERVIEW_VBLUR_3_ID - Vertical   blur for fourth split.
         * RENDERVIEW_HBLUR_3_ID - Horizontal blur for fourth split.
         * RENDERVIEW_DRAWSCENE_0_ID - Draw scene.
         * RENDERVIEW_DRAWSCENE_1_ID - Draw floor bottom.
         * RENDERVIEW_DRAWDEPTH_0_ID - Draw depth buffer for first  split.
         * RENDERVIEW_DRAWDEPTH_1_ID - Draw depth buffer for second split.
         * RENDERVIEW_DRAWDEPTH_2_ID - Draw depth buffer for third  split.
         * RENDERVIEW_DRAWDEPTH_3_ID - Draw depth buffer for fourth split.
         */

        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_2_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_3_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_4_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_VBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_HBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_VBLUR_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_HBLUR_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_VBLUR_2_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_HBLUR_2_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_VBLUR_3_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
        bgfx::setViewRect(RENDERVIEW_HBLUR_3_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_1_ID, lightView[0], lightProj[0]);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_2_ID, lightView[0], lightProj[1]);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_3_ID, lightView[0], lightProj[2]);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_4_ID, lightView[0], lightProj[3]);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_1_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_1_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_2_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_2_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_3_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_3_ID, screenView, screenProj);

        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_1_ID, s_rtShadowMap[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_2_ID, s_rtShadowMap[1]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_3_ID, s_rtShadowMap[2]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_4_ID, s_rtShadowMap[3]);
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_0_ID, s_rtBlur);         // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_0_ID, s_rtShadowMap[0]); // hblur
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_1_ID, s_rtBlur);         // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_1_ID, s_rtShadowMap[1]); // hblur
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_2_ID, s_rtBlur);         // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_2_ID, s_rtShadowMap[2]); // hblur
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_3_ID, s_rtBlur);         // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_3_ID, s_rtShadowMap[3]); // hblur
    }

    // Clear shadowmap rendertarget at beginning.
    const uint8_t flags0 = (LightType::DirectionalLight == m_settings.m_lightType)
                               ? 0
                               : BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;

    bgfx::setViewClear(RENDERVIEW_SHADOWMAP_0_ID,
                       flags0,
                       0xfefefefe // blur fails on completely white regions
                       ,
                       m_clearValues.m_clearDepth,
                       m_clearValues.m_clearStencil);
    bgfx::touch(RENDERVIEW_SHADOWMAP_0_ID);

    const uint8_t flags1 =
        (LightType::DirectionalLight == m_settings.m_lightType) ? BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH : 0;

    for(uint8_t ii = 0; ii < 4; ++ii)
    {
        bgfx::setViewClear(RENDERVIEW_SHADOWMAP_1_ID + ii,
                           flags1,
                           0xfefefefe // blur fails on completely white regions
                           ,
                           m_clearValues.m_clearDepth,
                           m_clearValues.m_clearStencil);
        bgfx::touch(RENDERVIEW_SHADOWMAP_1_ID + ii);
    }

    // Render.

    // Craft shadow map.
    {
        // Craft stencil mask for point light shadow map packing.
        if(LightType::PointLight == m_settings.m_lightType && m_settings.m_stencilPack)
        {
            if(6 == bgfx::getAvailTransientVertexBuffer(6, m_posLayout))
            {
                struct Pos
                {
                    float m_x, m_y, m_z;
                };

                bgfx::TransientVertexBuffer vb;
                bgfx::allocTransientVertexBuffer(&vb, 6, m_posLayout);
                Pos* vertex = (Pos*)vb.data;

                const float min = 0.0f;
                const float max = 1.0f;
                const float center = 0.5f;
                const float zz = 0.0f;

                vertex[0].m_x = min;
                vertex[0].m_y = min;
                vertex[0].m_z = zz;

                vertex[1].m_x = max;
                vertex[1].m_y = min;
                vertex[1].m_z = zz;

                vertex[2].m_x = center;
                vertex[2].m_y = center;
                vertex[2].m_z = zz;

                vertex[3].m_x = center;
                vertex[3].m_y = center;
                vertex[3].m_z = zz;

                vertex[4].m_x = max;
                vertex[4].m_y = max;
                vertex[4].m_z = zz;

                vertex[5].m_x = min;
                vertex[5].m_y = max;
                vertex[5].m_z = zz;

                bgfx::setState(0);
                bgfx::setStencil(BGFX_STENCIL_TEST_ALWAYS | BGFX_STENCIL_FUNC_REF(1) | BGFX_STENCIL_FUNC_RMASK(0xff) |
                                 BGFX_STENCIL_OP_FAIL_S_REPLACE | BGFX_STENCIL_OP_FAIL_Z_REPLACE |
                                 BGFX_STENCIL_OP_PASS_Z_REPLACE);
                bgfx::setVertexBuffer(0, &vb);
                bgfx::submit(RENDERVIEW_SHADOWMAP_0_ID, s_programs.m_black);
            }
        }

        render_scene_into_shadowmap(RENDERVIEW_SHADOWMAP_1_ID, models, currentSmSettings);
    }

    PackDepth::Enum depthType = (SmImpl::VSM == m_settings.m_smImpl) ? PackDepth::VSM : PackDepth::RGBA;
    bool bVsmOrEsm = (SmImpl::VSM == m_settings.m_smImpl) || (SmImpl::ESM == m_settings.m_smImpl);

    // Blur shadow map.
    if(bVsmOrEsm && currentSmSettings->m_doBlur)
    {
        bgfx::setTexture(4, s_shadowMap[0], bgfx::getTexture(s_rtShadowMap[0]));
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
        screenSpaceQuad(s_originBottomLeft);
        bgfx::submit(RENDERVIEW_VBLUR_0_ID, s_programs.m_vBlur[depthType]);

        bgfx::setTexture(4, s_shadowMap[0], bgfx::getTexture(s_rtBlur));
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
        screenSpaceQuad(s_originBottomLeft);
        bgfx::submit(RENDERVIEW_HBLUR_0_ID, s_programs.m_hBlur[depthType]);

        if(LightType::DirectionalLight == m_settings.m_lightType)
        {
            for(uint8_t ii = 1, jj = 2; ii < m_settings.m_numSplits; ++ii, jj += 2)
            {
                const uint8_t viewId = RENDERVIEW_VBLUR_0_ID + jj;

                bgfx::setTexture(4, s_shadowMap[0], bgfx::getTexture(s_rtShadowMap[ii]));
                bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
                screenSpaceQuad(s_originBottomLeft);
                bgfx::submit(viewId, s_programs.m_vBlur[depthType]);

                bgfx::setTexture(4, s_shadowMap[0], bgfx::getTexture(s_rtBlur));
                bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
                screenSpaceQuad(s_originBottomLeft);
                bgfx::submit(viewId + 1, s_programs.m_hBlur[depthType]);
            }
        }
    }

    // Draw scene.
    {
        // Setup shadow mtx.
        float mtxShadow[16];

        const float ymul = (s_originBottomLeft) ? 0.5f : -0.5f;
        float zadd = (DepthImpl::Linear == m_settings.m_depthImpl) ? 0.0f : 0.5f;

        // clang-format off
        const float mtxBias[16] =
        {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, ymul, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, zadd, 1.0f,
        };
        // clang-format on

        if(LightType::SpotLight == m_settings.m_lightType)
        {
            float mtxTmp[16];
            bx::mtxMul(mtxTmp, lightProj[ProjType::Horizontal], mtxBias);
            bx::mtxMul(mtxShadow, lightView[0], mtxTmp); // lightViewProjBias
        }
        else if(LightType::PointLight == m_settings.m_lightType)
        {
            const float s = (s_originBottomLeft) ? 1.0f : -1.0f; // sign
            zadd = (DepthImpl::Linear == m_settings.m_depthImpl) ? 0.0f : 0.5f;

            // clang-format off
            const float mtxCropBias[2][TetrahedronFaces::Count][16] =
            {
                { // settings.m_stencilPack == false

                 { // D3D: Green, OGL: Blue
                  0.25f,    0.0f, 0.0f, 0.0f,
                  0.0f, s*0.25f, 0.0f, 0.0f,
                  0.0f,    0.0f, 0.5f, 0.0f,
                  0.25f,   0.25f, zadd, 1.0f,
                  },
                 { // D3D: Yellow, OGL: Red
                  0.25f,    0.0f, 0.0f, 0.0f,
                  0.0f, s*0.25f, 0.0f, 0.0f,
                  0.0f,    0.0f, 0.5f, 0.0f,
                  0.75f,   0.25f, zadd, 1.0f,
                  },
                 { // D3D: Blue, OGL: Green
                  0.25f,    0.0f, 0.0f, 0.0f,
                  0.0f, s*0.25f, 0.0f, 0.0f,
                  0.0f,    0.0f, 0.5f, 0.0f,
                  0.25f,   0.75f, zadd, 1.0f,
                  },
                 { // D3D: Red, OGL: Yellow
                     0.25f,    0.0f, 0.0f, 0.0f,
                     0.0f, s*0.25f, 0.0f, 0.0f,
                     0.0f,    0.0f, 0.5f, 0.0f,
                     0.75f,   0.75f, zadd, 1.0f,
                     },
                 },
                { // settings.m_stencilPack == true

                 { // D3D: Red, OGL: Blue
                  0.25f,   0.0f, 0.0f, 0.0f,
                  0.0f, s*0.5f, 0.0f, 0.0f,
                  0.0f,   0.0f, 0.5f, 0.0f,
                  0.25f,   0.5f, zadd, 1.0f,
                  },
                 { // D3D: Blue, OGL: Red
                  0.25f,   0.0f, 0.0f, 0.0f,
                  0.0f, s*0.5f, 0.0f, 0.0f,
                  0.0f,   0.0f, 0.5f, 0.0f,
                  0.75f,   0.5f, zadd, 1.0f,
                  },
                 { // D3D: Green, OGL: Green
                  0.5f,    0.0f, 0.0f, 0.0f,
                  0.0f, s*0.25f, 0.0f, 0.0f,
                  0.0f,    0.0f, 0.5f, 0.0f,
                  0.5f,   0.75f, zadd, 1.0f,
                  },
                 { // D3D: Yellow, OGL: Yellow
                     0.5f,    0.0f, 0.0f, 0.0f,
                     0.0f, s*0.25f, 0.0f, 0.0f,
                     0.0f,    0.0f, 0.5f, 0.0f,
                     0.5f,   0.25f, zadd, 1.0f,
                     },
                 }
            };
            // clang-format on

            // clang-format off
            // Use as: [stencilPack][flipV][tetrahedronFace]
            static const uint8_t cropBiasIndices[2][2][4] =
            {
                { // settings.m_stencilPack == false
                    { 0, 1, 2, 3 }, //flipV == false
                    { 2, 3, 0, 1 }, //flipV == true
                },
                { // settings.m_stencilPack == true
                    { 3, 2, 0, 1 }, //flipV == false
                    { 2, 3, 0, 1 }, //flipV == true
                },
            };
            // clang-format on

            for(uint8_t ii = 0; ii < TetrahedronFaces::Count; ++ii)
            {
                ProjType::Enum projType = (m_settings.m_stencilPack) ? ProjType::Enum(ii > 1) : ProjType::Horizontal;
                uint8_t biasIndex = cropBiasIndices[m_settings.m_stencilPack][uint8_t(s_originBottomLeft)][ii];

                float mtxTmp[16];
                bx::mtxMul(mtxTmp, mtxYpr[ii], lightProj[projType]);
                bx::mtxMul(m_shadowMapMtx[ii],
                           mtxTmp,
                           mtxCropBias[m_settings.m_stencilPack][biasIndex]); // mtxYprProjBias
            }

            bx::mtxTranslate(mtxShadow // lightInvTranslate
                             ,
                             -m_pointLight.m_position.m_v[0],
                             -m_pointLight.m_position.m_v[1],
                             -m_pointLight.m_position.m_v[2]);
        }
        else // LightType::DirectionalLight == settings.m_lightType
        {
            for(uint8_t ii = 0; ii < m_settings.m_numSplits; ++ii)
            {
                float mtxTmp[16];

                bx::mtxMul(mtxTmp, lightProj[ii], mtxBias);
                bx::mtxMul(m_shadowMapMtx[ii], lightView[0], mtxTmp); // lViewProjCropBias
            }
        }

        if(LightType::DirectionalLight != m_settings.m_lightType)
        {
            float tmp[16];
            bx::mtxIdentity(tmp);

            bx::mtxMul(m_lightMtx, tmp, mtxShadow);
        }
    }
}

void shadow::render_scene_into_shadowmap(uint8_t shadowmap_1_id,
                                         const shadow_map_models_t& models,
                                         ShadowMapSettings* currentSmSettings)
{
    // Draw scene into shadowmap.
    uint8_t drawNum;
    if(LightType::SpotLight == m_settings.m_lightType)
    {
        drawNum = 1;
    }
    else if(LightType::PointLight == m_settings.m_lightType)
    {
        drawNum = 4;
    }
    else // LightType::DirectionalLight == settings.m_lightType)
    {
        drawNum = uint8_t(m_settings.m_numSplits);
    }

    for(const auto& e : models)
    {
        const auto& transform_comp = e.get<transform_component>();
        const auto& model_comp = e.get<model_component>();

        const auto& model = model_comp.get_model();
        if(!model.is_valid())
            continue;

        const auto& world_transform = transform_comp.get_transform_global();

        const auto current_lod_index = 0;
        const auto current_mesh = model.get_lod(current_lod_index);
        if(!current_mesh)
            continue;

        for(uint8_t ii = 0; ii < drawNum; ++ii)
        {
            const uint8_t viewId = shadowmap_1_id + ii;

            uint8_t renderStateIndex = RenderState::ShadowMap_PackDepth;
            if(LightType::PointLight == m_settings.m_lightType && m_settings.m_stencilPack)
            {
                renderStateIndex =
                    uint8_t((ii < 2) ? RenderState::ShadowMap_PackDepthHoriz : RenderState::ShadowMap_PackDepthVert);
            }

            const auto& _renderState = s_renderStates[renderStateIndex];

            // Set uniforms.
            // s_uniforms.submitPerDrawUniforms();

            const auto& bone_transforms = model_comp.get_bone_transforms();
            model.submit(viewId,
                         world_transform,
                         bone_transforms,
                         current_lod_index,
                         currentSmSettings->m_progPack,
                         currentSmSettings->m_progPack,
                         [&]()
                         {
                             s_uniforms.submitPerDrawUniforms();

                             // Apply render state.
                             bgfx::setStencil(_renderState.m_fstencil, _renderState.m_bstencil);
                             bgfx::setState(_renderState.m_state, _renderState.m_blendFactorRgba);
                         });
        }
    }
}

void Programs::init(rtti::context& ctx)
{
    auto& am = ctx.get<asset_manager>();

    auto loadProgram = [&](const std::string& vs, const std::string& fs)
    {
        auto vs_shader = am.get_asset<gfx::shader>("engine:/data/shaders/shadowmaps/" + vs + ".sc");
        auto fs_shadfer = am.get_asset<gfx::shader>("engine:/data/shaders/shadowmaps/" + fs + ".sc");

        m_programs.emplace_back(std::make_shared<gpu_program>(vs_shader, fs_shadfer));
        return m_programs.back()->native_handle();
    };

    auto loadProgramPtr = [&](const std::string& vs, const std::string& fs)
    {
        auto vs_shader = am.get_asset<gfx::shader>("engine:/data/shaders/" + vs + ".sc");
        auto fs_shadfer = am.get_asset<gfx::shader>("engine:/data/shaders/" + fs + ".sc");

        m_programs.emplace_back(std::make_shared<gpu_program>(vs_shader, fs_shadfer));
        return m_programs.back();
    };

    // clang-format off
     // Misc.
    m_black        = loadProgram("vs_shadowmaps_color",         "fs_shadowmaps_color_black");

    // Blur.
    m_vBlur[PackDepth::RGBA] = loadProgram("vs_shadowmaps_vblur", "fs_shadowmaps_vblur");
    m_hBlur[PackDepth::RGBA] = loadProgram("vs_shadowmaps_hblur", "fs_shadowmaps_hblur");
    m_vBlur[PackDepth::VSM]  = loadProgram("vs_shadowmaps_vblur", "fs_shadowmaps_vblur_vsm");
    m_hBlur[PackDepth::VSM]  = loadProgram("vs_shadowmaps_hblur", "fs_shadowmaps_hblur_vsm");

    // Draw depth.
    m_drawDepth[PackDepth::RGBA] = loadProgram("vs_shadowmaps_unpackdepth", "fs_shadowmaps_unpackdepth");
    m_drawDepth[PackDepth::VSM]  = loadProgram("vs_shadowmaps_unpackdepth", "fs_shadowmaps_unpackdepth_vsm");

    // Pack depth.
    m_packDepth[DepthImpl::InvZ][PackDepth::RGBA] = loadProgramPtr("shadowmaps/vs_shadowmaps_packdepth", "shadowmaps/fs_shadowmaps_packdepth");
    m_packDepth[DepthImpl::InvZ][PackDepth::VSM]  = loadProgramPtr("shadowmaps/vs_shadowmaps_packdepth", "shadowmaps/fs_shadowmaps_packdepth_vsm");

    m_packDepth[DepthImpl::Linear][PackDepth::RGBA] = loadProgramPtr("shadowmaps/vs_shadowmaps_packdepth_linear", "shadowmaps/fs_shadowmaps_packdepth_linear");
    m_packDepth[DepthImpl::Linear][PackDepth::VSM]  = loadProgramPtr("shadowmaps/vs_shadowmaps_packdepth_linear", "shadowmaps/fs_shadowmaps_packdepth_vsm_linear");

    // Color lighting.
    m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::Hard] = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_hard");
    m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::PCF]  = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_pcf");
    m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::VSM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_vsm");
    m_colorLighting[SmType::Single][DepthImpl::InvZ][SmImpl::ESM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_esm");

    m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::Hard] = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_hard_linear");
    m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::PCF]  = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_pcf_linear");
    m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::VSM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_vsm_linear");
    m_colorLighting[SmType::Single][DepthImpl::Linear][SmImpl::ESM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_spot_light_esm_linear");

    m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::Hard] = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_hard");
    m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::PCF]  = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_pcf");
    m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::VSM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_vsm");
    m_colorLighting[SmType::Omni][DepthImpl::InvZ][SmImpl::ESM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_esm");

    m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::Hard] = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_hard_linear");
    m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::PCF]  = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_pcf_linear");
    m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::VSM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_vsm_linear");
    m_colorLighting[SmType::Omni][DepthImpl::Linear][SmImpl::ESM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_point_light_esm_linear");

    m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::Hard] = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_hard");
    m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::PCF]  = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_pcf");
    m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::VSM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_vsm");
    m_colorLighting[SmType::Cascade][DepthImpl::InvZ][SmImpl::ESM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_esm");

    m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::Hard] = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_hard_linear");
    m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::PCF]  = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_pcf_linear");
    m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::VSM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_vsm_linear");
    m_colorLighting[SmType::Cascade][DepthImpl::Linear][SmImpl::ESM]  = loadProgramPtr("vs_clip_quad", "fs_deferred_directional_light_esm_linear");
    // clang-format on
}

} // namespace ace
