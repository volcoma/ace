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

/**
 * _splits = { near0, far0, near1, far1... nearN, farN }
 * N = _numSplits
 */
void splitFrustum(float* _splits, uint8_t _numSplits, float _near, float _far, float _splitWeight = 0.75f)
{
    auto factor = float(_numSplits) / 4.0f;
    _far = _far * factor;

    APPLOG_INFO("split_frustum near {}, far {}", _near, _far);
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

shadowmap_generator::shadowmap_generator()
{
    init(engine::context());
}

shadowmap_generator::~shadowmap_generator()
{
    deinit();
}

void shadowmap_generator::deinit()
{
    deinit_uniforms();
    deinit_textures();
}

void shadowmap_generator::deinit_textures()
{
    if(!valid_)
    {
        return;
    }

    valid_ = false;

    for(int i = 0; i < ShadowMapRenderTargets::Count; ++i)
    {
        if(bgfx::isValid(rt_shadow_map_[i]))
        {
            bgfx::destroy(rt_shadow_map_[i]);
            rt_shadow_map_[i] = {bgfx::kInvalidHandle};
        }
    }

    if(bgfx::isValid(rt_blur_))
    {
        bgfx::destroy(rt_blur_);
        rt_blur_ = {bgfx::kInvalidHandle};
    }
}

void shadowmap_generator::deinit_uniforms()
{
    if(bgfx::isValid(tex_color_))
    {
        bgfx::destroy(tex_color_);
    }

    for(int i = 0; i < ShadowMapRenderTargets::Count; ++i)
    {
        if(bgfx::isValid(shadow_map_[i]))
        {
            bgfx::destroy(shadow_map_[i]);
        }
    }
}

void shadowmap_generator::init(rtti::context& ctx)
{
    if(bgfx::isValid(tex_color_))
    {
        return;
    }
    // Uniforms.
    uniforms_.init();
    tex_color_ = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
    shadow_map_[0] = bgfx::createUniform("s_shadowMap0", bgfx::UniformType::Sampler);
    shadow_map_[1] = bgfx::createUniform("s_shadowMap1", bgfx::UniformType::Sampler);
    shadow_map_[2] = bgfx::createUniform("s_shadowMap2", bgfx::UniformType::Sampler);
    shadow_map_[3] = bgfx::createUniform("s_shadowMap3", bgfx::UniformType::Sampler);

    for(int i = 0; i < ShadowMapRenderTargets::Count; ++i)
    {
        rt_shadow_map_[i] = {bgfx::kInvalidHandle};
    }

    // Programs.
    programs_.init(ctx);

    // Vertex declarations.
    pos_layout_.begin();
    pos_layout_.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
    pos_layout_.end();

    PosColorTexCoord0Vertex::init();

    // Lights.
    // clang-format off
    point_light_ =
        {
            { { 0.0f, 0.0f, 0.0f, 1.0f   } }, //position
            { { 0.0f,-0.4f,-0.6f, 0.0f   } }, //spotdirection, spotexponent
        };

    directional_light_ =
        {
            { { 0.5f,-1.0f, 0.1f, 0.0f  } }, //position
            { { 0.0f, 0.0f, 0.0f, 1.0f  } }, //spotdirection, spotexponent
        };

    // clang-format on

    // Setup uniforms.
    color_[0] = color_[1] = color_[2] = color_[3] = 1.0f;
    uniforms_.setPtrs(&point_light_,
                      color_,
                      light_mtx_,
                      &shadow_map_mtx_[ShadowMapRenderTargets::First][0],
                      &shadow_map_mtx_[ShadowMapRenderTargets::Second][0],
                      &shadow_map_mtx_[ShadowMapRenderTargets::Third][0],
                      &shadow_map_mtx_[ShadowMapRenderTargets::Fourth][0]);
    uniforms_.submitConstUniforms();

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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPackSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPackSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::VSM].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::VSM].get() //m_progPackSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPackSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::VSM].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::VSM].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::VSM].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::VSM].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::VSM].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::VSM].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::VSM].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::VSM].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::InvZ][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::VSM].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::VSM].get() //m_packDepthSkinned
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
                    , programs_.m_packDepth[DepthImpl::Linear][PackDepth::RGBA].get() //m_progPack
                    , programs_.m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA].get() //m_packDepthSkinned
                }

            }
        }
    };
    // clang-format on
    bx::memCopy(sm_settings_, smSettings, sizeof(smSettings));

    settings_.m_lightType = LightType::SpotLight;
    settings_.m_depthImpl = DepthImpl::InvZ;
    settings_.m_smImpl = SmImpl::Hard;
    settings_.m_spotOuterAngle = 45.0f;
    settings_.m_spotInnerAngle = 30.0f;
    settings_.m_fovXAdjust = 0.0f;
    settings_.m_fovYAdjust = 0.0f;
    settings_.m_coverageSpotL = 90.0f;
    settings_.m_splitDistribution = 0.6f;
    settings_.m_numSplits = 4;
    settings_.m_updateLights = true;
    settings_.m_updateScene = true;
    settings_.m_drawDepthBuffer = false;
    settings_.m_showSmCoverage = false;
    settings_.m_stencilPack = true;
    settings_.m_stabilize = true;
}

auto shadowmap_generator::get_depth_type() const -> PackDepth::Enum
{
    PackDepth::Enum depthType = (SmImpl::VSM == settings_.m_smImpl) ? PackDepth::VSM : PackDepth::RGBA;
    return depthType;
}

auto shadowmap_generator::get_rt_texture(uint8_t split) const -> bgfx::TextureHandle
{
    if(!bgfx::isValid(shadow_map_[split]))
    {
        return {bgfx::kInvalidHandle};
    }

    return bgfx::getTexture(rt_shadow_map_[split]);
}

auto shadowmap_generator::get_depth_render_program(PackDepth::Enum depth) const -> bgfx::ProgramHandle
{
    return programs_.m_drawDepth[depth]->native_handle();
}

void shadowmap_generator::submit_uniforms(uint8_t stage) const
{
    if(!bgfx::isValid(tex_color_))
    {
        return;
    }
    uniforms_.submitPerDrawUniforms();

    for(uint8_t ii = 0; ii < ShadowMapRenderTargets::Count; ++ii)
    {
        if(!bgfx::isValid(rt_shadow_map_[ii]))
        {
            continue;
        }

        bgfx::setTexture(stage + ii, shadow_map_[ii], bgfx::getTexture(rt_shadow_map_[ii]));
    }
}

void shadowmap_generator::update(const light& l, const math::transform& ltrans)
{
    if(l.casts_shadows == false)
    {
        deinit_textures();
        return;
    }

    bool recreateTextures = false;
    recreateTextures |= !valid_;

    valid_ = true;

    const auto& pos = ltrans.get_position();
    const auto& dir = ltrans.z_unit_axis();
    point_light_.m_position.m_x = pos.x;
    point_light_.m_position.m_y = pos.y;
    point_light_.m_position.m_z = pos.z;

    point_light_.m_spotDirectionInner.m_x = dir.x;
    point_light_.m_spotDirectionInner.m_y = dir.y;
    point_light_.m_spotDirectionInner.m_z = dir.z;

    directional_light_.m_position.m_x = dir.x;
    directional_light_.m_position.m_y = dir.y;
    directional_light_.m_position.m_z = dir.z;

    auto ltype = convert(l.type);
    recreateTextures |= ltype != settings_.m_lightType;

    settings_.m_lightType = ltype;
    settings_.m_smImpl = convert(l.shadow_params.type);
    settings_.m_depthImpl = convert(l.shadow_params.depth);

    settings_.m_showSmCoverage = l.shadow_params.show_coverage;

    switch(l.type)
    {
        case light_type::spot:
            settings_.m_spotOuterAngle = l.spot_data.get_outer_angle();
            settings_.m_spotInnerAngle = l.spot_data.get_inner_angle();
            settings_.m_coverageSpotL = settings_.m_spotOuterAngle;
            break;
        case light_type::point:
            settings_.m_stencilPack = l.point_data.shadow_params.stencil_pack;
            settings_.m_fovXAdjust = l.point_data.shadow_params.fov_x_adjust;
            settings_.m_fovYAdjust = l.point_data.shadow_params.fov_y_adjust;

            break;
        default:

            settings_.m_splitDistribution = l.directional_data.shadow_params.split_distribution;
            settings_.m_numSplits = l.directional_data.shadow_params.num_splits;
            settings_.m_stabilize = l.directional_data.shadow_params.stabilize;

            break;
    }

    ShadowMapSettings* currentSmSettings =
        &sm_settings_[settings_.m_lightType][settings_.m_depthImpl][settings_.m_smImpl];

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
    uniforms_.m_shadowMapBias = currentSmSettings->m_bias;
    uniforms_.m_shadowMapOffset = currentSmSettings->m_normalOffset;
    uniforms_.m_shadowMapParam0 = currentSmSettings->m_customParam0;
    uniforms_.m_shadowMapParam1 = currentSmSettings->m_customParam1;
    uniforms_.m_depthValuePow = currentSmSettings->m_depthValuePow;
    uniforms_.m_XNum = currentSmSettings->m_xNum;
    uniforms_.m_YNum = currentSmSettings->m_yNum;
    uniforms_.m_XOffset = currentSmSettings->m_xOffset;
    uniforms_.m_YOffset = currentSmSettings->m_yOffset;
    uniforms_.m_showSmCoverage = float(settings_.m_showSmCoverage);
    uniforms_.m_lightPtr = (LightType::DirectionalLight == settings_.m_lightType) ? &directional_light_ : &point_light_;

    if(LightType::SpotLight == settings_.m_lightType)
    {
        point_light_.m_spotDirectionInner.m_inner = settings_.m_spotInnerAngle;
    }

    // Update render target size.
    uint16_t shadowMapSize = 1 << uint32_t(currentSmSettings->m_sizePwrTwo);
    recreateTextures |= current_shadow_map_size_ != shadowMapSize;

    if(recreateTextures)
    {
        current_shadow_map_size_ = shadowMapSize;

        if(bgfx::isValid(rt_shadow_map_[0]))
        {
            bgfx::destroy(rt_shadow_map_[0]);
        }

        {
            bgfx::TextureHandle fbtextures[] = {
                bgfx::createTexture2D(current_shadow_map_size_,
                                      current_shadow_map_size_,
                                      false,
                                      1,
                                      bgfx::TextureFormat::BGRA8,
                                      BGFX_TEXTURE_RT),
                bgfx::createTexture2D(current_shadow_map_size_,
                                      current_shadow_map_size_,
                                      false,
                                      1,
                                      bgfx::TextureFormat::D24S8,
                                      BGFX_TEXTURE_RT),
            };
            rt_shadow_map_[0] = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
        }

        if(LightType::DirectionalLight == settings_.m_lightType)
        {
            for(uint8_t ii = 1; ii < ShadowMapRenderTargets::Count; ++ii)
            {
                if(bgfx::isValid(rt_shadow_map_[ii]))
                {
                    bgfx::destroy(rt_shadow_map_[ii]);
                }

                {
                    bgfx::TextureHandle fbtextures[] = {
                        bgfx::createTexture2D(current_shadow_map_size_,
                                              current_shadow_map_size_,
                                              false,
                                              1,
                                              bgfx::TextureFormat::BGRA8,
                                              BGFX_TEXTURE_RT),
                        bgfx::createTexture2D(current_shadow_map_size_,
                                              current_shadow_map_size_,
                                              false,
                                              1,
                                              bgfx::TextureFormat::D24S8,
                                              BGFX_TEXTURE_RT),
                    };
                    rt_shadow_map_[ii] = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
                }
            }
        }

        if(bgfx::isValid(rt_blur_))
        {
            bgfx::destroy(rt_blur_);
        }
        rt_blur_ =
            bgfx::createFrameBuffer(current_shadow_map_size_, current_shadow_map_size_, bgfx::TextureFormat::BGRA8);
    }

    float currentShadowMapSizef = float(int16_t(current_shadow_map_size_));
    uniforms_.m_shadowMapTexelSize = 1.0f / currentShadowMapSizef;
}

void shadowmap_generator::generate_shadowmaps(const shadow_map_models_t& models, const camera* cam)
{
    ShadowMapSettings* currentSmSettings =
        &sm_settings_[settings_.m_lightType][settings_.m_depthImpl][settings_.m_smImpl];

    /// begin generating
    gfx::render_pass shadowmap_pass_0("shadowmap_pass_0");
    gfx::render_pass shadowmap_pass_1("shadowmap_pass_1");
    gfx::render_pass shadowmap_pass_2("shadowmap_pass_2");
    gfx::render_pass shadowmap_pass_3("shadowmap_pass_3");
    gfx::render_pass shadowmap_pass_4("shadowmap_pass_4");
    gfx::render_pass shadowmap_vblur_pass_0("shadowmap_vblur_pass_0");
    gfx::render_pass shadowmap_hblur_pass_0("shadowmap_hblur_pass_0");
    gfx::render_pass shadowmap_vblur_pass_1("shadowmap_hblur_pass_1");
    gfx::render_pass shadowmap_hblur_pass_1("shadowmap_hblur_pass_1");
    gfx::render_pass shadowmap_vblur_pass_2("shadowmap_vblur_pass_2");
    gfx::render_pass shadowmap_hblur_pass_2("shadowmap_hblur_pass_2");
    gfx::render_pass shadowmap_vblur_pass_3("shadowmap_vblur_pass_3");
    gfx::render_pass shadowmap_hblur_pass_3("shadowmap_hblur_pass_3");

    auto RENDERVIEW_SHADOWMAP_0_ID = shadowmap_pass_0.id;
    auto RENDERVIEW_SHADOWMAP_1_ID = shadowmap_pass_1.id;
    auto RENDERVIEW_SHADOWMAP_2_ID = shadowmap_pass_2.id;
    auto RENDERVIEW_SHADOWMAP_3_ID = shadowmap_pass_3.id;
    auto RENDERVIEW_SHADOWMAP_4_ID = shadowmap_pass_4.id;
    auto RENDERVIEW_VBLUR_0_ID = shadowmap_vblur_pass_0.id;
    auto RENDERVIEW_HBLUR_0_ID = shadowmap_hblur_pass_0.id;
    auto RENDERVIEW_VBLUR_1_ID = shadowmap_vblur_pass_1.id;
    auto RENDERVIEW_HBLUR_1_ID = shadowmap_hblur_pass_1.id;
    auto RENDERVIEW_VBLUR_2_ID = shadowmap_vblur_pass_2.id;
    auto RENDERVIEW_HBLUR_2_ID = shadowmap_hblur_pass_2.id;
    auto RENDERVIEW_VBLUR_3_ID = shadowmap_vblur_pass_3.id;
    auto RENDERVIEW_HBLUR_3_ID = shadowmap_hblur_pass_3.id;

    bool homogeneousDepth = gfx::is_homogeneous_depth();
    bool originBottomLeft = gfx::is_origin_bottom_left();

    // Compute transform matrices.
    const uint8_t shadowMapPasses = ShadowMapRenderTargets::Count;
    float lightView[shadowMapPasses][16];
    float lightProj[shadowMapPasses][16];

    math::frustum lightFrustums[shadowMapPasses];

    float mtxYpr[TetrahedronFaces::Count][16];

    float screenProj[16];
    float screenView[16];
    bx::mtxIdentity(screenView);

    bx::mtxOrtho(screenProj, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 100.0f, 0.0f, homogeneousDepth);

    if(LightType::SpotLight == settings_.m_lightType)
    {
        const float fovy = settings_.m_coverageSpotL;
        const float aspect = 1.0f;
        bx::mtxProj(lightProj[ProjType::Horizontal],
                    fovy,
                    aspect,
                    currentSmSettings->m_near,
                    currentSmSettings->m_far,
                    false);

        // For linear depth, prevent depth division by variable w-component in shaders and divide here by far plane
        if(DepthImpl::Linear == settings_.m_depthImpl)
        {
            lightProj[ProjType::Horizontal][10] /= currentSmSettings->m_far;
            lightProj[ProjType::Horizontal][14] /= currentSmSettings->m_far;
        }

        const bx::Vec3 at = bx::add(bx::load<bx::Vec3>(point_light_.m_position.m_v),
                                    bx::load<bx::Vec3>(point_light_.m_spotDirectionInner.m_v));
        bx::mtxLookAt(lightView[TetrahedronFaces::Green], bx::load<bx::Vec3>(point_light_.m_position.m_v), at);
    }
    else if(LightType::PointLight == settings_.m_lightType)
    {
        float ypr[TetrahedronFaces::Count][3] = {
            {bx::toRad(0.0f), bx::toRad(27.36780516f), bx::toRad(0.0f)},
            {bx::toRad(180.0f), bx::toRad(27.36780516f), bx::toRad(0.0f)},
            {bx::toRad(-90.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f)},
            {bx::toRad(90.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f)},
        };

        if(settings_.m_stencilPack)
        {
            const float fovx = 143.98570868f + 3.51f + settings_.m_fovXAdjust;
            const float fovy = 125.26438968f + 9.85f + settings_.m_fovYAdjust;
            const float aspect = bx::tan(bx::toRad(fovx * 0.5f)) / bx::tan(bx::toRad(fovy * 0.5f));

            bx::mtxProj(lightProj[ProjType::Vertical],
                        fovx,
                        aspect,
                        currentSmSettings->m_near,
                        currentSmSettings->m_far,
                        false);

            // For linear depth, prevent depth division by variable w-component in shaders and divide here by far plane
            if(DepthImpl::Linear == settings_.m_depthImpl)
            {
                lightProj[ProjType::Vertical][10] /= currentSmSettings->m_far;
                lightProj[ProjType::Vertical][14] /= currentSmSettings->m_far;
            }

            ypr[TetrahedronFaces::Green][2] = bx::toRad(180.0f);
            ypr[TetrahedronFaces::Yellow][2] = bx::toRad(0.0f);
            ypr[TetrahedronFaces::Blue][2] = bx::toRad(90.0f);
            ypr[TetrahedronFaces::Red][2] = bx::toRad(-90.0f);
        }

        const float fovx = 143.98570868f + 7.8f + settings_.m_fovXAdjust;
        const float fovy = 125.26438968f + 3.0f + settings_.m_fovYAdjust;
        const float aspect = bx::tan(bx::toRad(fovx * 0.5f)) / bx::tan(bx::toRad(fovy * 0.5f));

        bx::mtxProj(lightProj[ProjType::Horizontal],
                    fovy,
                    aspect,
                    currentSmSettings->m_near,
                    currentSmSettings->m_far,
                    homogeneousDepth);

        // For linear depth, prevent depth division by variable w component in shaders and divide here by far plane
        if(DepthImpl::Linear == settings_.m_depthImpl)
        {
            lightProj[ProjType::Horizontal][10] /= currentSmSettings->m_far;
            lightProj[ProjType::Horizontal][14] /= currentSmSettings->m_far;
        }

        for(uint8_t ii = 0; ii < TetrahedronFaces::Count; ++ii)
        {
            float mtxTmp[16];
            mtxYawPitchRoll(mtxTmp, ypr[ii][0], ypr[ii][1], ypr[ii][2]);

            float tmp[3] = {
                -bx::dot(bx::load<bx::Vec3>(point_light_.m_position.m_v), bx::load<bx::Vec3>(&mtxTmp[0])),
                -bx::dot(bx::load<bx::Vec3>(point_light_.m_position.m_v), bx::load<bx::Vec3>(&mtxTmp[4])),
                -bx::dot(bx::load<bx::Vec3>(point_light_.m_position.m_v), bx::load<bx::Vec3>(&mtxTmp[8])),
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
            -directional_light_.m_position.m_x,
            -directional_light_.m_position.m_y,
            -directional_light_.m_position.m_z,
        };
        const bx::Vec3 at = {0.0f, 0.0f, 0.0f};
        // const bx::Vec3 at = bx::mul(eye, 100);
        bx::mtxLookAt(lightView[0], eye, at);

        // Compute split distances.
        const uint8_t maxNumSplits = 4;
        BX_ASSERT(maxNumSplits >= settings_.m_numSplits, "Error! Max num splits.");

        // Split distances

        std::array<float, maxNumSplits * 2> splitSlices; //[maxNumSplits * 2];
        splitFrustum(splitSlices.data(),
                     uint8_t(settings_.m_numSplits),
                     currentSmSettings->m_near,
                     currentSmSettings->m_far,
                     settings_.m_splitDistribution);

        float mtxProj[16];
        bx::mtxOrtho(mtxProj,
                     1.0f,
                     -1.0f,
                     1.0f,
                     -1.0f,
                     -currentSmSettings->m_far,
                     currentSmSettings->m_far,
                     0.0f,
                     homogeneousDepth);

        // Update uniforms.
        for(uint8_t ii = 0, ff = 1; ii < settings_.m_numSplits; ++ii, ff += 2)
        {
            // This lags for 1 frame, but it's not a problem.
            uniforms_.m_csmFarDistances[ii] = splitSlices[ff];
        }

        // Compute camera inverse view mtx.

        // Define a fixed scene bounding box (min and max corners in world space)
        math::bbox scene_bounds{{-5.0f, -5.0f, -5.0f}, {5.0f, 5.0f, 5.0f}};
        float mtxViewInv[16];

        if(!cam)
        {
            for(const auto& e : models)
            {
                auto bounds = defaults::calc_bounds(e);

                scene_bounds.add_point(bounds.min);
                scene_bounds.add_point(bounds.max);
            }
        }
        else
        {
            bx::mtxInverse(mtxViewInv, cam->get_view());
        }

        const uint8_t numCorners = 8;
        float frustumCorners[maxNumSplits][numCorners][3];
        for(uint8_t ii = 0, nn = 0, ff = 1; ii < settings_.m_numSplits; ++ii, nn += 2, ff += 2)
        {
            bx::Vec3 min = {9000.0f, 9000.0f, 9000.0f};
            bx::Vec3 max = {-9000.0f, -9000.0f, -9000.0f};
            if(cam)
            {
                const float camFovy = cam->get_fov();
                const float camAspect = cam->get_aspect_ratio();
                const float projHeight = bx::tan(bx::toRad(camFovy) * 0.5f);
                const float projWidth = projHeight * camAspect;

                // Compute frustum corners for one split in world space.
                worldSpaceFrustumCorners((float*)frustumCorners[ii],
                                         splitSlices[nn],
                                         splitSlices[ff],
                                         projWidth,
                                         projHeight,
                                         mtxViewInv);

                for(uint8_t jj = 0; jj < numCorners; ++jj)
                {
                    // Transform to light space.
                    const bx::Vec3 xyz = bx::mul(bx::load<bx::Vec3>(frustumCorners[ii][jj]), lightView[0]);

                    // Update bounding box.
                    min = bx::min(min, xyz);
                    max = bx::max(max, xyz);
                }
            }
            else
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

                for(uint8_t jj = 0; jj < 8; ++jj)
                {
                    // Transform to light space
                    bx::Vec3 lightSpaceCorner = bx::mul(corners[jj], lightView[0]);

                    // Update bounding box in light space
                    min = bx::min(min, lightSpaceCorner);
                    max = bx::max(max, lightSpaceCorner);
                }
            }

            const bx::Vec3 minproj = bx::mulH(min, mtxProj);
            const bx::Vec3 maxproj = bx::mulH(max, mtxProj);

            float scalex = 2.0f / (maxproj.x - minproj.x);
            float scaley = 2.0f / (maxproj.y - minproj.y);

            if(settings_.m_stabilize)
            {
                const float quantizer = 64.0f;
                scalex = quantizer / bx::ceil(quantizer / scalex);
                scaley = quantizer / bx::ceil(quantizer / scaley);
            }

            float offsetx = 0.5f * (maxproj.x + minproj.x) * scalex;
            float offsety = 0.5f * (maxproj.y + minproj.y) * scaley;

            if(settings_.m_stabilize)
            {
                float currentShadowMapSizef = float(int16_t(current_shadow_map_size_));
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

    if(LightType::SpotLight == settings_.m_lightType)
    {
        /**
         * RENDERVIEW_SHADOWMAP_0_ID - Clear shadow map. (used as convenience, otherwise render_pass_1 could be cleared)
         * RENDERVIEW_SHADOWMAP_1_ID - Craft shadow map.
         * RENDERVIEW_VBLUR_0_ID - Vertical blur.
         * RENDERVIEW_HBLUR_0_ID - Horizontal blur.
         */

        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_VBLUR_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_HBLUR_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);

        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_1_ID, lightView[0], lightProj[ProjType::Horizontal]);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_0_ID, screenView, screenProj);

        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_0_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_1_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_0_ID, rt_blur_);
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_0_ID, rt_shadow_map_[0]);

        lightFrustums[0].update(math::make_mat4(lightView[0]), math::make_mat4(lightProj[ProjType::Horizontal]), false);
    }
    else if(LightType::PointLight == settings_.m_lightType)
    {
        /**
         * RENDERVIEW_SHADOWMAP_0_ID - Clear entire shadow map.
         * RENDERVIEW_SHADOWMAP_1_ID - Craft green tetrahedron shadow face.
         * RENDERVIEW_SHADOWMAP_2_ID - Craft yellow tetrahedron shadow face.
         * RENDERVIEW_SHADOWMAP_3_ID - Craft blue tetrahedron shadow face.
         * RENDERVIEW_SHADOWMAP_4_ID - Craft red tetrahedron shadow face.
         * RENDERVIEW_VBLUR_0_ID - Vertical blur.
         * RENDERVIEW_HBLUR_0_ID - Horizontal blur.
         */

        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        if(settings_.m_stencilPack)
        {
            const uint16_t f = current_shadow_map_size_;     // full size
            const uint16_t h = current_shadow_map_size_ / 2; // half size
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, f, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_2_ID, 0, h, f, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_3_ID, 0, 0, h, f);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_4_ID, h, 0, h, f);
        }
        else
        {
            const uint16_t h = current_shadow_map_size_ / 2; // half size
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, h, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_2_ID, h, 0, h, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_3_ID, 0, h, h, h);
            bgfx::setViewRect(RENDERVIEW_SHADOWMAP_4_ID, h, h, h, h);
        }
        bgfx::setViewRect(RENDERVIEW_VBLUR_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_HBLUR_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);

        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_1_ID,
                               lightView[TetrahedronFaces::Green],
                               lightProj[ProjType::Horizontal]);

        lightFrustums[TetrahedronFaces::Green].update(math::make_mat4(lightView[TetrahedronFaces::Green]),
                                                      math::make_mat4(lightProj[ProjType::Horizontal]),
                                                      false);

        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_2_ID,
                               lightView[TetrahedronFaces::Yellow],
                               lightProj[ProjType::Horizontal]);

        lightFrustums[TetrahedronFaces::Yellow].update(math::make_mat4(lightView[TetrahedronFaces::Yellow]),
                                                       math::make_mat4(lightProj[ProjType::Horizontal]),
                                                       false);

        if(settings_.m_stencilPack)
        {
            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_3_ID,
                                   lightView[TetrahedronFaces::Blue],
                                   lightProj[ProjType::Vertical]);

            lightFrustums[TetrahedronFaces::Blue].update(math::make_mat4(lightView[TetrahedronFaces::Blue]),
                                                         math::make_mat4(lightProj[ProjType::Vertical]),
                                                         false);

            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_4_ID,
                                   lightView[TetrahedronFaces::Red],
                                   lightProj[ProjType::Vertical]);

            lightFrustums[TetrahedronFaces::Red].update(math::make_mat4(lightView[TetrahedronFaces::Red]),
                                                        math::make_mat4(lightProj[ProjType::Vertical]),
                                                        false);
        }
        else
        {
            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_3_ID,
                                   lightView[TetrahedronFaces::Blue],
                                   lightProj[ProjType::Horizontal]);

            lightFrustums[TetrahedronFaces::Blue].update(math::make_mat4(lightView[TetrahedronFaces::Blue]),
                                                         math::make_mat4(lightProj[ProjType::Horizontal]),
                                                         false);

            bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_4_ID,
                                   lightView[TetrahedronFaces::Red],
                                   lightProj[ProjType::Horizontal]);

            lightFrustums[TetrahedronFaces::Red].update(math::make_mat4(lightView[TetrahedronFaces::Red]),
                                                        math::make_mat4(lightProj[ProjType::Horizontal]),
                                                        false);
        }
        bgfx::setViewTransform(RENDERVIEW_VBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_0_ID, screenView, screenProj);

        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_0_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_1_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_2_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_3_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_4_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_0_ID, rt_blur_);
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_0_ID, rt_shadow_map_[0]);
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
         */

        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_1_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_2_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_3_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_SHADOWMAP_4_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_VBLUR_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_HBLUR_0_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_VBLUR_1_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_HBLUR_1_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_VBLUR_2_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_HBLUR_2_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_VBLUR_3_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);
        bgfx::setViewRect(RENDERVIEW_HBLUR_3_ID, 0, 0, current_shadow_map_size_, current_shadow_map_size_);

        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_1_ID, lightView[0], lightProj[0]);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_2_ID, lightView[0], lightProj[1]);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_3_ID, lightView[0], lightProj[2]);
        bgfx::setViewTransform(RENDERVIEW_SHADOWMAP_4_ID, lightView[0], lightProj[3]);

        lightFrustums[0].update(math::make_mat4(lightView[0]), math::make_mat4(lightProj[0]), false);
        lightFrustums[1].update(math::make_mat4(lightView[0]), math::make_mat4(lightProj[1]), false);
        lightFrustums[2].update(math::make_mat4(lightView[0]), math::make_mat4(lightProj[2]), false);
        lightFrustums[3].update(math::make_mat4(lightView[0]), math::make_mat4(lightProj[3]), false);

        bgfx::setViewTransform(RENDERVIEW_VBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_0_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_1_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_1_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_2_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_2_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_VBLUR_3_ID, screenView, screenProj);
        bgfx::setViewTransform(RENDERVIEW_HBLUR_3_ID, screenView, screenProj);

        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_1_ID, rt_shadow_map_[0]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_2_ID, rt_shadow_map_[1]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_3_ID, rt_shadow_map_[2]);
        bgfx::setViewFrameBuffer(RENDERVIEW_SHADOWMAP_4_ID, rt_shadow_map_[3]);
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_0_ID, rt_blur_);          // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_0_ID, rt_shadow_map_[0]); // hblur
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_1_ID, rt_blur_);          // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_1_ID, rt_shadow_map_[1]); // hblur
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_2_ID, rt_blur_);          // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_2_ID, rt_shadow_map_[2]); // hblur
        bgfx::setViewFrameBuffer(RENDERVIEW_VBLUR_3_ID, rt_blur_);          // vblur
        bgfx::setViewFrameBuffer(RENDERVIEW_HBLUR_3_ID, rt_shadow_map_[3]); // hblur
    }

    // Clear shadowmap rendertarget at beginning.
    const uint8_t flags0 = (LightType::DirectionalLight == settings_.m_lightType)
                               ? 0
                               : BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;

    bgfx::setViewClear(RENDERVIEW_SHADOWMAP_0_ID,
                       flags0,
                       0xfefefefe // blur fails on completely white regions
                       ,
                       clear_values_.clear_depth,
                       clear_values_.clear_stencil);
    bgfx::touch(RENDERVIEW_SHADOWMAP_0_ID);

    const uint8_t flags1 =
        (LightType::DirectionalLight == settings_.m_lightType) ? BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH : 0;

    for(uint8_t ii = 0; ii < 4; ++ii)
    {
        bgfx::setViewClear(RENDERVIEW_SHADOWMAP_1_ID + ii,
                           flags1,
                           0xfefefefe // blur fails on completely white regions
                           ,
                           clear_values_.clear_depth,
                           clear_values_.clear_stencil);
        bgfx::touch(RENDERVIEW_SHADOWMAP_1_ID + ii);
    }

    // Render.

    uniforms_.submitPerFrameUniforms();

    // Craft shadow map.
    {
        // Craft stencil mask for point light shadow map packing.
        if(LightType::PointLight == settings_.m_lightType && settings_.m_stencilPack)
        {
            if(6 == bgfx::getAvailTransientVertexBuffer(6, pos_layout_))
            {
                struct Pos
                {
                    float m_x, m_y, m_z;
                };

                bgfx::TransientVertexBuffer vb;
                bgfx::allocTransientVertexBuffer(&vb, 6, pos_layout_);
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

                programs_.m_black->begin();
                bgfx::submit(RENDERVIEW_SHADOWMAP_0_ID, programs_.m_black->native_handle());
                programs_.m_black->end();
            }
        }

        render_scene_into_shadowmap(RENDERVIEW_SHADOWMAP_1_ID, models, lightFrustums, currentSmSettings);
    }

    PackDepth::Enum depthType = (SmImpl::VSM == settings_.m_smImpl) ? PackDepth::VSM : PackDepth::RGBA;
    bool bVsmOrEsm = (SmImpl::VSM == settings_.m_smImpl) || (SmImpl::ESM == settings_.m_smImpl);

    // Blur shadow map.
    if(bVsmOrEsm && currentSmSettings->m_doBlur)
    {
        bgfx::setTexture(4, shadow_map_[0], bgfx::getTexture(rt_shadow_map_[0]));
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
        screenSpaceQuad(originBottomLeft);
        programs_.m_vBlur[depthType]->begin();
        bgfx::submit(RENDERVIEW_VBLUR_0_ID, programs_.m_vBlur[depthType]->native_handle());
        programs_.m_vBlur[depthType]->end();

        bgfx::setTexture(4, shadow_map_[0], bgfx::getTexture(rt_blur_));
        bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
        screenSpaceQuad(originBottomLeft);
        programs_.m_hBlur[depthType]->begin();
        bgfx::submit(RENDERVIEW_HBLUR_0_ID, programs_.m_hBlur[depthType]->native_handle());
        programs_.m_hBlur[depthType]->end();

        if(LightType::DirectionalLight == settings_.m_lightType)
        {
            for(uint8_t ii = 1, jj = 2; ii < settings_.m_numSplits; ++ii, jj += 2)
            {
                const uint8_t viewId = RENDERVIEW_VBLUR_0_ID + jj;

                bgfx::setTexture(4, shadow_map_[0], bgfx::getTexture(rt_shadow_map_[ii]));
                bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
                screenSpaceQuad(originBottomLeft);
                bgfx::submit(viewId, programs_.m_vBlur[depthType]->native_handle());

                bgfx::setTexture(4, shadow_map_[0], bgfx::getTexture(rt_blur_));
                bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
                screenSpaceQuad(originBottomLeft);
                bgfx::submit(viewId + 1, programs_.m_hBlur[depthType]->native_handle());
            }
        }
    }

    // Draw scene.
    {
        // Setup shadow mtx.
        float mtxShadow[16];

        const float ymul = (originBottomLeft) ? 0.5f : -0.5f;
        float zadd = (DepthImpl::Linear == settings_.m_depthImpl) ? 0.0f : 0.5f;

        // clang-format off
        const float mtxBias[16] =
        {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, ymul, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, zadd, 1.0f,
        };
        // clang-format on

        if(LightType::SpotLight == settings_.m_lightType)
        {
            float mtxTmp[16];
            bx::mtxMul(mtxTmp, lightProj[ProjType::Horizontal], mtxBias);
            bx::mtxMul(mtxShadow, lightView[0], mtxTmp); // lightViewProjBias
        }
        else if(LightType::PointLight == settings_.m_lightType)
        {
            const float s = (originBottomLeft) ? 1.0f : -1.0f; // sign
            zadd = (DepthImpl::Linear == settings_.m_depthImpl) ? 0.0f : 0.5f;

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
                ProjType::Enum projType = (settings_.m_stencilPack) ? ProjType::Enum(ii > 1) : ProjType::Horizontal;
                uint8_t biasIndex = cropBiasIndices[settings_.m_stencilPack][uint8_t(originBottomLeft)][ii];

                float mtxTmp[16];
                bx::mtxMul(mtxTmp, mtxYpr[ii], lightProj[projType]);
                bx::mtxMul(shadow_map_mtx_[ii],
                           mtxTmp,
                           mtxCropBias[settings_.m_stencilPack][biasIndex]); // mtxYprProjBias
            }

            bx::mtxTranslate(mtxShadow // lightInvTranslate
                             ,
                             -point_light_.m_position.m_v[0],
                             -point_light_.m_position.m_v[1],
                             -point_light_.m_position.m_v[2]);
        }
        else // LightType::DirectionalLight == settings.m_lightType
        {
            for(uint8_t ii = 0; ii < settings_.m_numSplits; ++ii)
            {
                float mtxTmp[16];

                bx::mtxMul(mtxTmp, lightProj[ii], mtxBias);
                bx::mtxMul(shadow_map_mtx_[ii], lightView[0], mtxTmp); // lViewProjCropBias
            }
        }

        if(LightType::DirectionalLight != settings_.m_lightType)
        {
            float tmp[16];
            bx::mtxIdentity(tmp);

            bx::mtxMul(light_mtx_, tmp, mtxShadow);
        }
    }
}

void shadowmap_generator::render_scene_into_shadowmap(uint8_t shadowmap_1_id,
                                                      const shadow_map_models_t& models,
                                                      const math::frustum lightFrustums[ShadowMapRenderTargets::Count],
                                                      ShadowMapSettings* currentSmSettings)
{
    // Draw scene into shadowmap.
    uint8_t drawNum;
    if(LightType::SpotLight == settings_.m_lightType)
    {
        drawNum = 1;
    }
    else if(LightType::PointLight == settings_.m_lightType)
    {
        drawNum = 4;
    }
    else // LightType::DirectionalLight == settings.m_lightType)
    {
        drawNum = uint8_t(settings_.m_numSplits);
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
        const auto lod = model.get_lod(current_lod_index);
        if(!lod)
            continue;

        const auto& mesh = lod.get();

        auto bounds = mesh->get_bounds();

        for(uint8_t ii = 0; ii < drawNum; ++ii)
        {
            const uint8_t viewId = shadowmap_1_id + ii;

            uint8_t renderStateIndex = RenderState::ShadowMap_PackDepth;
            if(LightType::PointLight == settings_.m_lightType && settings_.m_stencilPack)
            {
                renderStateIndex =
                    uint8_t((ii < 2) ? RenderState::ShadowMap_PackDepthHoriz : RenderState::ShadowMap_PackDepthVert);
            }

            const auto& _renderState = render_states_[renderStateIndex];

            if(!lightFrustums[ii].test_obb(bounds, world_transform))
            {
                continue;
            }

            const auto& bone_transforms = model_comp.get_bone_transforms();

            model::submit_callbacks callbacks;
            callbacks.setup_begin = [&](const model::submit_callbacks::params& submit_params)
            {
                auto& prog =
                    submit_params.skinned ? currentSmSettings->m_progPackSkinned : currentSmSettings->m_progPack;
                prog->begin();
            };
            callbacks.setup_params_per_instance = [&](const model::submit_callbacks::params& submit_params)
            {
                // Set uniforms.
                uniforms_.submitPerDrawUniforms();

                // Apply render state.
                gfx::set_stencil(_renderState.m_fstencil, _renderState.m_bstencil);
                gfx::set_state(_renderState.m_state, _renderState.m_blendFactorRgba);
            };
            callbacks.setup_params_per_subset =
                [&](const model::submit_callbacks::params& submit_params, const material& mat)
            {
                auto& prog =
                    submit_params.skinned ? currentSmSettings->m_progPackSkinned : currentSmSettings->m_progPack;

                gfx::submit(viewId, prog->native_handle());
            };
            callbacks.setup_end = [&](const model::submit_callbacks::params& submit_params)
            {
                auto& prog =
                    submit_params.skinned ? currentSmSettings->m_progPackSkinned : currentSmSettings->m_progPack;

                prog->end();
            };

            model.submit(world_transform, bone_transforms, current_lod_index, callbacks);
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

        return std::make_shared<gpu_program>(vs_shader, fs_shadfer);
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
    m_packDepth[DepthImpl::InvZ][PackDepth::RGBA] = loadProgram("vs_shadowmaps_packdepth", "fs_shadowmaps_packdepth");
    m_packDepth[DepthImpl::InvZ][PackDepth::VSM]  = loadProgram("vs_shadowmaps_packdepth", "fs_shadowmaps_packdepth_vsm");

    m_packDepth[DepthImpl::Linear][PackDepth::RGBA] = loadProgram("vs_shadowmaps_packdepth_linear", "fs_shadowmaps_packdepth_linear");
    m_packDepth[DepthImpl::Linear][PackDepth::VSM]  = loadProgram("vs_shadowmaps_packdepth_linear", "fs_shadowmaps_packdepth_vsm_linear");

    m_packDepthSkinned[DepthImpl::InvZ][PackDepth::RGBA] = loadProgram("vs_shadowmaps_packdepth_skinned", "fs_shadowmaps_packdepth");
    m_packDepthSkinned[DepthImpl::InvZ][PackDepth::VSM]  = loadProgram("vs_shadowmaps_packdepth_skinned", "fs_shadowmaps_packdepth_vsm");

    m_packDepthSkinned[DepthImpl::Linear][PackDepth::RGBA] = loadProgram("vs_shadowmaps_packdepth_linear_skinned", "fs_shadowmaps_packdepth_linear");
    m_packDepthSkinned[DepthImpl::Linear][PackDepth::VSM]  = loadProgram("vs_shadowmaps_packdepth_linear_skinned", "fs_shadowmaps_packdepth_vsm_linear");

}

} // namespace ace
