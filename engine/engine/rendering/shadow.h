#pragma once
#include <engine/ecs/ecs.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/light.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/rendering/gpu_program.h>
#include <graphics/graphics.h>

namespace ace
{

struct LightType
{
    enum Enum
    {
        SpotLight,
        PointLight,
        DirectionalLight,

        Count
    };
};

struct DepthImpl
{
    enum Enum
    {
        InvZ,
        Linear,

        Count
    };
};

struct PackDepth
{
    enum Enum
    {
        RGBA,
        VSM,

        Count
    };
};

struct SmImpl
{
    enum Enum
    {
        Hard,
        PCF,
        VSM,
        ESM,

        Count
    };
};

struct SmType
{
    enum Enum
    {
        Single,
        Omni,
        Cascade,

        Count
    };
};

struct TetrahedronFaces
{
    enum Enum
    {
        Green,
        Yellow,
        Blue,
        Red,

        Count
    };
};

struct ProjType
{
    enum Enum
    {
        Horizontal,
        Vertical,

        Count
    };
};

struct ShadowMapRenderTargets
{
    enum Enum
    {
        First,
        Second,
        Third,
        Fourth,

        Count
    };
};

struct PosNormalTexcoordVertex
{
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_normal;
    float m_u;
    float m_v;
};

struct Light
{
    union Position
    {
        struct
        {
            float m_x;
            float m_y;
            float m_z;
            float m_w;
        };

        float m_v[4];
    };

    union SpotDirectionInner
    {
        struct
        {
            float m_x;
            float m_y;
            float m_z;
            float m_inner;
        };

        float m_v[4];
    };

    Position m_position;
    SpotDirectionInner m_spotDirectionInner;
};

struct Uniforms
{
    void init()
    {
        m_ambientPass = 1.0f;
        m_lightingPass = 1.0f;

        m_shadowMapBias = 0.003f;
        m_shadowMapOffset = 0.0f;
        m_shadowMapParam0 = 0.5;
        m_shadowMapParam1 = 1.0;
        m_depthValuePow = 1.0f;
        m_showSmCoverage = 1.0f;
        m_shadowMapTexelSize = 1.0f / 512.0f;

        m_csmFarDistances[0] = 30.0f;
        m_csmFarDistances[1] = 90.0f;
        m_csmFarDistances[2] = 180.0f;
        m_csmFarDistances[3] = 1000.0f;

        m_tetraNormalGreen[0] = 0.0f;
        m_tetraNormalGreen[1] = -0.57735026f;
        m_tetraNormalGreen[2] = 0.81649661f;

        m_tetraNormalYellow[0] = 0.0f;
        m_tetraNormalYellow[1] = -0.57735026f;
        m_tetraNormalYellow[2] = -0.81649661f;

        m_tetraNormalBlue[0] = -0.81649661f;
        m_tetraNormalBlue[1] = 0.57735026f;
        m_tetraNormalBlue[2] = 0.0f;

        m_tetraNormalRed[0] = 0.81649661f;
        m_tetraNormalRed[1] = 0.57735026f;
        m_tetraNormalRed[2] = 0.0f;

        m_XNum = 2.0f;
        m_YNum = 2.0f;
        m_XOffset = 10.0f / 512.0f;
        m_YOffset = 10.0f / 512.0f;

        u_params0 = bgfx::createUniform("u_params0", bgfx::UniformType::Vec4);
        u_params1 = bgfx::createUniform("u_params1", bgfx::UniformType::Vec4);
        u_params2 = bgfx::createUniform("u_params2", bgfx::UniformType::Vec4);
        u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
        u_smSamplingParams = bgfx::createUniform("u_smSamplingParams", bgfx::UniformType::Vec4);
        u_csmFarDistances = bgfx::createUniform("u_csmFarDistances", bgfx::UniformType::Vec4);
        u_lightMtx = bgfx::createUniform("u_lightMtx", bgfx::UniformType::Mat4);

        u_tetraNormalGreen = bgfx::createUniform("u_tetraNormalGreen", bgfx::UniformType::Vec4);
        u_tetraNormalYellow = bgfx::createUniform("u_tetraNormalYellow", bgfx::UniformType::Vec4);
        u_tetraNormalBlue = bgfx::createUniform("u_tetraNormalBlue", bgfx::UniformType::Vec4);
        u_tetraNormalRed = bgfx::createUniform("u_tetraNormalRed", bgfx::UniformType::Vec4);

        u_shadowMapMtx0 = bgfx::createUniform("u_shadowMapMtx0", bgfx::UniformType::Mat4);
        u_shadowMapMtx1 = bgfx::createUniform("u_shadowMapMtx1", bgfx::UniformType::Mat4);
        u_shadowMapMtx2 = bgfx::createUniform("u_shadowMapMtx2", bgfx::UniformType::Mat4);
        u_shadowMapMtx3 = bgfx::createUniform("u_shadowMapMtx3", bgfx::UniformType::Mat4);

        u_lightPosition = bgfx::createUniform("u_lightPosition", bgfx::UniformType::Vec4);
        u_lightAmbientPower = bgfx::createUniform("u_lightAmbientPower", bgfx::UniformType::Vec4);
        u_lightDiffusePower = bgfx::createUniform("u_lightDiffusePower", bgfx::UniformType::Vec4);
        u_lightSpecularPower = bgfx::createUniform("u_lightSpecularPower", bgfx::UniformType::Vec4);
        u_lightSpotDirectionInner = bgfx::createUniform("u_lightSpotDirectionInner", bgfx::UniformType::Vec4);
        u_lightAttenuationSpotOuter = bgfx::createUniform("u_lightAttenuationSpotOuter", bgfx::UniformType::Vec4);

        u_materialKa = bgfx::createUniform("u_materialKa", bgfx::UniformType::Vec4);
        u_materialKd = bgfx::createUniform("u_materialKd", bgfx::UniformType::Vec4);
        u_materialKs = bgfx::createUniform("u_materialKs", bgfx::UniformType::Vec4);
    }

    void setPtrs(Light* _lightPtr,
                 float* _colorPtr,
                 float* _lightMtxPtr,
                 float* _shadowMapMtx0,
                 float* _shadowMapMtx1,
                 float* _shadowMapMtx2,
                 float* _shadowMapMtx3)
    {
        m_lightMtxPtr = _lightMtxPtr;
        m_colorPtr = _colorPtr;
        m_lightPtr = _lightPtr;

        m_shadowMapMtx0 = _shadowMapMtx0;
        m_shadowMapMtx1 = _shadowMapMtx1;
        m_shadowMapMtx2 = _shadowMapMtx2;
        m_shadowMapMtx3 = _shadowMapMtx3;
    }

    // Call this once at initialization.
    void submitConstUniforms() const
    {
        bgfx::setUniform(u_tetraNormalGreen, m_tetraNormalGreen);
        bgfx::setUniform(u_tetraNormalYellow, m_tetraNormalYellow);
        bgfx::setUniform(u_tetraNormalBlue, m_tetraNormalBlue);
        bgfx::setUniform(u_tetraNormalRed, m_tetraNormalRed);
    }

    // Call this once per frame.
    void submitPerFrameUniforms() const
    {
        bgfx::setUniform(u_params1, m_params1);
        bgfx::setUniform(u_params2, m_params2);
        bgfx::setUniform(u_smSamplingParams, m_paramsBlur);
        bgfx::setUniform(u_csmFarDistances, m_csmFarDistances);
    }

    // Call this before each draw call.
    void submitPerDrawUniforms() const
    {
        bgfx::setUniform(u_shadowMapMtx0, m_shadowMapMtx0);
        bgfx::setUniform(u_shadowMapMtx1, m_shadowMapMtx1);
        bgfx::setUniform(u_shadowMapMtx2, m_shadowMapMtx2);
        bgfx::setUniform(u_shadowMapMtx3, m_shadowMapMtx3);

        bgfx::setUniform(u_params0, m_params0);
        bgfx::setUniform(u_lightMtx, m_lightMtxPtr);
        bgfx::setUniform(u_color, m_colorPtr);
    }

    void destroy()
    {
        bgfx::destroy(u_params0);
        bgfx::destroy(u_params1);
        bgfx::destroy(u_params2);
        bgfx::destroy(u_color);
        bgfx::destroy(u_smSamplingParams);
        bgfx::destroy(u_csmFarDistances);

        bgfx::destroy(u_materialKa);
        bgfx::destroy(u_materialKd);
        bgfx::destroy(u_materialKs);

        bgfx::destroy(u_tetraNormalGreen);
        bgfx::destroy(u_tetraNormalYellow);
        bgfx::destroy(u_tetraNormalBlue);
        bgfx::destroy(u_tetraNormalRed);

        bgfx::destroy(u_shadowMapMtx0);
        bgfx::destroy(u_shadowMapMtx1);
        bgfx::destroy(u_shadowMapMtx2);
        bgfx::destroy(u_shadowMapMtx3);

        bgfx::destroy(u_lightMtx);
        bgfx::destroy(u_lightPosition);
        bgfx::destroy(u_lightAmbientPower);
        bgfx::destroy(u_lightDiffusePower);
        bgfx::destroy(u_lightSpecularPower);
        bgfx::destroy(u_lightSpotDirectionInner);
        bgfx::destroy(u_lightAttenuationSpotOuter);
    }

    union
    {
        struct
        {
            float m_ambientPass;
            float m_lightingPass;
            float m_unused00;
            float m_unused01;
        };

        float m_params0[4];
    };

    union
    {
        struct
        {
            float m_shadowMapBias;
            float m_shadowMapOffset;
            float m_shadowMapParam0;
            float m_shadowMapParam1;
        };

        float m_params1[4];
    };

    union
    {
        struct
        {
            float m_depthValuePow;
            float m_showSmCoverage;
            float m_shadowMapTexelSize;
            float m_unused23;
        };

        float m_params2[4];
    };

    union
    {
        struct
        {
            float m_XNum;
            float m_YNum;
            float m_XOffset;
            float m_YOffset;
        };

        float m_paramsBlur[4];
    };

    float m_tetraNormalGreen[3];
    float m_tetraNormalYellow[3];
    float m_tetraNormalBlue[3];
    float m_tetraNormalRed[3];
    float m_csmFarDistances[4];

    float* m_lightMtxPtr;
    float* m_colorPtr;
    Light* m_lightPtr;
    float* m_shadowMapMtx0;
    float* m_shadowMapMtx1;
    float* m_shadowMapMtx2;
    float* m_shadowMapMtx3;

private:
    bgfx::UniformHandle u_params0;
    bgfx::UniformHandle u_params1;
    bgfx::UniformHandle u_params2;
    bgfx::UniformHandle u_color;
    bgfx::UniformHandle u_smSamplingParams;
    bgfx::UniformHandle u_csmFarDistances;

    bgfx::UniformHandle u_materialKa;
    bgfx::UniformHandle u_materialKd;
    bgfx::UniformHandle u_materialKs;

    bgfx::UniformHandle u_tetraNormalGreen;
    bgfx::UniformHandle u_tetraNormalYellow;
    bgfx::UniformHandle u_tetraNormalBlue;
    bgfx::UniformHandle u_tetraNormalRed;

    bgfx::UniformHandle u_shadowMapMtx0;
    bgfx::UniformHandle u_shadowMapMtx1;
    bgfx::UniformHandle u_shadowMapMtx2;
    bgfx::UniformHandle u_shadowMapMtx3;

    bgfx::UniformHandle u_lightMtx;
    bgfx::UniformHandle u_lightPosition;
    bgfx::UniformHandle u_lightAmbientPower;
    bgfx::UniformHandle u_lightDiffusePower;
    bgfx::UniformHandle u_lightSpecularPower;
    bgfx::UniformHandle u_lightSpotDirectionInner;
    bgfx::UniformHandle u_lightAttenuationSpotOuter;
};

struct RenderState
{
    enum Enum
    {
        Default = 0,

        ShadowMap_PackDepth,
        ShadowMap_PackDepthHoriz,
        ShadowMap_PackDepthVert,

        Custom_BlendLightTexture,
        Custom_DrawPlaneBottom,

        Count
    };

    uint64_t m_state;
    uint32_t m_blendFactorRgba;
    uint32_t m_fstencil;
    uint32_t m_bstencil;
};

struct PosColorTexCoord0Vertex
{
    float m_x;
    float m_y;
    float m_z;
    uint32_t m_rgba;
    float m_u;
    float m_v;

    static void init()
    {
        ms_layout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .end();
    }

    static bgfx::VertexLayout ms_layout;
};

struct Programs
{
    void init(rtti::context& ctx);

    void destroy()
    {
        m_programs.clear();

        // Color lighting.
        for(uint8_t ii = 0; ii < SmType::Count; ++ii)
        {
            for(uint8_t jj = 0; jj < DepthImpl::Count; ++jj)
            {
                for(uint8_t kk = 0; kk < SmImpl::Count; ++kk)
                {
                    m_colorLighting[ii][jj][kk].reset();
                }
            }
        }

        // Pack depth.
        for(uint8_t ii = 0; ii < DepthImpl::Count; ++ii)
        {
            for(uint8_t jj = 0; jj < PackDepth::Count; ++jj)
            {
                m_packDepth[ii][jj].reset();
            }
        }

        // Draw depth.
        for(uint8_t ii = 0; ii < PackDepth::Count; ++ii)
        {
            bgfx::destroy(m_drawDepth[ii]);
        }

        // Hblur.
        for(uint8_t ii = 0; ii < PackDepth::Count; ++ii)
        {
            bgfx::destroy(m_hBlur[ii]);
        }

        // Vblur.
        for(uint8_t ii = 0; ii < PackDepth::Count; ++ii)
        {
            bgfx::destroy(m_vBlur[ii]);
        }

        // Misc.
        bgfx::destroy(m_black);
    }

    bgfx::ProgramHandle m_black;
    bgfx::ProgramHandle m_vBlur[PackDepth::Count];
    bgfx::ProgramHandle m_hBlur[PackDepth::Count];
    bgfx::ProgramHandle m_drawDepth[PackDepth::Count];
    std::shared_ptr<gpu_program> m_packDepth[DepthImpl::Count][PackDepth::Count];
    std::shared_ptr<gpu_program> m_packDepthSkinned[DepthImpl::Count][PackDepth::Count];
    std::shared_ptr<gpu_program> m_colorLighting[SmType::Count][DepthImpl::Count][SmImpl::Count];
    std::shared_ptr<gpu_program> m_colorLightingNoop[LightType::Count];
    std::vector<std::shared_ptr<gpu_program>> m_programs;
};

struct ShadowMapSettings
{
#define IMGUI_FLOAT_PARAM(_name) float _name, _name##Min, _name##Max, _name##Step
    IMGUI_FLOAT_PARAM(m_sizePwrTwo);
    IMGUI_FLOAT_PARAM(m_depthValuePow);
    IMGUI_FLOAT_PARAM(m_near);
    IMGUI_FLOAT_PARAM(m_far);
    IMGUI_FLOAT_PARAM(m_bias);
    IMGUI_FLOAT_PARAM(m_normalOffset);
    IMGUI_FLOAT_PARAM(m_customParam0);
    IMGUI_FLOAT_PARAM(m_customParam1);
    IMGUI_FLOAT_PARAM(m_xNum);
    IMGUI_FLOAT_PARAM(m_yNum);
    IMGUI_FLOAT_PARAM(m_xOffset);
    IMGUI_FLOAT_PARAM(m_yOffset);
    bool m_doBlur;
    gpu_program* m_progPack;
    gpu_program* m_progPackSkinned;
    gpu_program* m_progDraw;
#undef IMGUI_FLOAT_PARAM
};

struct SceneSettings
{
    LightType::Enum m_lightType;
    DepthImpl::Enum m_depthImpl;
    SmImpl::Enum m_smImpl;
    float m_spotOuterAngle;
    float m_spotInnerAngle;
    float m_fovXAdjust;
    float m_fovYAdjust;
    float m_coverageSpotL;
    float m_splitDistribution;
    int m_numSplits;
    bool m_updateLights;
    bool m_updateScene;
    bool m_drawDepthBuffer;
    bool m_showSmCoverage;
    bool m_stencilPack;
    bool m_stabilize;
};

struct ClearValues
{
    ClearValues(uint32_t _clearRgba = 0x30303000, float _clearDepth = 1.0f, uint8_t _clearStencil = 0)
        : clear_rgba(_clearRgba)
        , clear_depth(_clearDepth)
        , clear_stencil(_clearStencil)
    {
    }

    uint32_t clear_rgba;
    float clear_depth;
    uint8_t clear_stencil;
};

using shadow_map_models_t = std::vector<entt::handle>;

class shadowmap_generator
{
public:
    shadowmap_generator();

    void init(rtti::context& ctx);

    void generate_shadowmaps(const light& l,
                             const math::transform& ltrans,
                             const shadow_map_models_t& model,
                             const camera* cam = nullptr);

    auto get_depth_type() const -> PackDepth::Enum;
    auto get_rt_texture(uint8_t split) const -> bgfx::TextureHandle;
    auto get_depth_render_program(PackDepth::Enum depth) const -> bgfx::ProgramHandle;
    auto get_color_apply_program(const light& l) const -> gpu_program*;
    void submit_uniforms() const;

private:
    void render_scene_into_shadowmap(uint8_t shadowmap_1_id,
                                     const shadow_map_models_t& models,
                                     const math::frustum frustums[ShadowMapRenderTargets::Count],
                                     ShadowMapSettings* currentSmSettings);

    // clang-format off
    RenderState render_states_[RenderState::Count] =
    {
        { // Default
            0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
            , UINT32_MAX
            , BGFX_STENCIL_NONE
            , BGFX_STENCIL_NONE
        },
        { // ShadowMap_PackDepth
            0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
            , UINT32_MAX
            , BGFX_STENCIL_NONE
            , BGFX_STENCIL_NONE
        },
        { // ShadowMap_PackDepthHoriz
            0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
            , UINT32_MAX
            , BGFX_STENCIL_TEST_EQUAL
                | BGFX_STENCIL_FUNC_REF(1)
                | BGFX_STENCIL_FUNC_RMASK(0xff)
                | BGFX_STENCIL_OP_FAIL_S_KEEP
                | BGFX_STENCIL_OP_FAIL_Z_KEEP
                | BGFX_STENCIL_OP_PASS_Z_KEEP
            , BGFX_STENCIL_NONE
        },
        { // ShadowMap_PackDepthVert
            0
                | BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
            , UINT32_MAX
            , BGFX_STENCIL_TEST_EQUAL
                | BGFX_STENCIL_FUNC_REF(0)
                | BGFX_STENCIL_FUNC_RMASK(0xff)
                | BGFX_STENCIL_OP_FAIL_S_KEEP
                | BGFX_STENCIL_OP_FAIL_Z_KEEP
                | BGFX_STENCIL_OP_PASS_Z_KEEP
            , BGFX_STENCIL_NONE
        },
        { // Custom_BlendLightTexture
            BGFX_STATE_WRITE_RGB
                | BGFX_STATE_WRITE_A
                | BGFX_STATE_WRITE_Z
                | BGFX_STATE_DEPTH_TEST_LESS
                | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_COLOR, BGFX_STATE_BLEND_INV_SRC_COLOR)
                | BGFX_STATE_CULL_CCW
                | BGFX_STATE_MSAA
            , UINT32_MAX
            , BGFX_STENCIL_NONE
            , BGFX_STENCIL_NONE
        },
        { // Custom_DrawPlaneBottom
            BGFX_STATE_WRITE_RGB
                | BGFX_STATE_CULL_CW
                | BGFX_STATE_MSAA
            , UINT32_MAX
            , BGFX_STENCIL_NONE
            , BGFX_STENCIL_NONE
        },
    };
    // clang-format on

    ClearValues clear_values_;

    bgfx::VertexLayout pos_layout_;

    float color_[4];
    Light point_light_;
    Light directional_light_;

    float light_mtx_[16];
    float shadow_map_mtx_[ShadowMapRenderTargets::Count][16];

    ShadowMapSettings sm_settings_[LightType::Count][DepthImpl::Count][SmImpl::Count];
    SceneSettings settings_;

    uint16_t current_shadow_map_size_{};

    Uniforms uniforms_;
    Programs programs_;

    bgfx::UniformHandle tex_color_{bgfx::kInvalidHandle};
    bgfx::UniformHandle shadow_map_[ShadowMapRenderTargets::Count];
    bgfx::FrameBufferHandle rt_shadow_map_[ShadowMapRenderTargets::Count];
    bgfx::FrameBufferHandle rt_blur_{bgfx::kInvalidHandle};

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
