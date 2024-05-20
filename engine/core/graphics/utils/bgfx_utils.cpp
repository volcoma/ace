/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "common.h"

#include <tinystl/allocator.h>
#include <tinystl/string.h>
#include <tinystl/vector.h>
namespace stl = tinystl;

#include "entry/entry.h"
#include <bgfx/bgfx.h>
#include <bx/commandline.h>
#include <bx/endian.h>
#include <bx/math.h>
#include <bx/readerwriter.h>
#include <bx/string.h>

#include "bgfx_utils.h"

#include <bimg/decode.h>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/file.h>
#include <bx/sort.h>

#include <time.h>
namespace entry
{
namespace
{

bx::AllocatorI* getDefaultAllocator()
{
    static bx::DefaultAllocator allocator;
    return &allocator;
}

static bx::AllocatorI* g_allocator = getDefaultAllocator();
typedef bx::StringT<&g_allocator> String;

class FileReader : public bx::FileReader
{
    typedef bx::FileReader super;

public:
    virtual bool open(const bx::FilePath& _filePath, bx::Error* _err) override
    {
        return super::open(_filePath, _err);
    }
};

class FileWriter : public bx::FileWriter
{
    typedef bx::FileWriter super;

public:
    virtual bool open(const bx::FilePath& _filePath, bool _append, bx::Error* _err) override
    {
        return super::open(_filePath, _append, _err);
    }
};

bx::AllocatorI* getAllocator()
{
    return g_allocator;
}

} // namespace
} // namespace entry

void* load(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
{
    if(bx::open(_reader, _filePath))
    {
        uint32_t size = (uint32_t)bx::getSize(_reader);
        void* data = bx::alloc(_allocator, size);
        bx::read(_reader, data, size, bx::ErrorAssert{});
        bx::close(_reader);
        if(NULL != _size)
        {
            *_size = size;
        }
        return data;
    }
    else
    {
        DBG("Failed to open: %s.", _filePath);
    }

    if(NULL != _size)
    {
        *_size = 0;
    }

    return NULL;
}

void* load(const char* _filePath, uint32_t* _size)
{
    entry::FileReader reader;
    return load(&reader, entry::getAllocator(), _filePath, _size);
}

void unload(void* _ptr)
{
    bx::free(entry::getAllocator(), _ptr);
}

static const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath)
{
    if(bx::open(_reader, _filePath))
    {
        uint32_t size = (uint32_t)bx::getSize(_reader);
        const bgfx::Memory* mem = bgfx::alloc(size + 1);
        bx::read(_reader, mem->data, size, bx::ErrorAssert{});
        bx::close(_reader);
        mem->data[mem->size - 1] = '\0';
        return mem;
    }

    DBG("Failed to load %s.", _filePath);
    return NULL;
}

static void* loadMem(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size)
{
    if(bx::open(_reader, _filePath))
    {
        uint32_t size = (uint32_t)bx::getSize(_reader);
        void* data = bx::alloc(_allocator, size);
        bx::read(_reader, data, size, bx::ErrorAssert{});
        bx::close(_reader);

        if(NULL != _size)
        {
            *_size = size;
        }
        return data;
    }

    DBG("Failed to load %s.", _filePath);
    return NULL;
}

static bgfx::ShaderHandle loadShader(bx::FileReaderI* _reader, const char* _name)
{
    char filePath[512];

    const char* shaderPath = "???";

    switch(bgfx::getRendererType())
    {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12:
            shaderPath = "shaders/dx11/";
            break;
        case bgfx::RendererType::Agc:
        case bgfx::RendererType::Gnm:
            shaderPath = "shaders/pssl/";
            break;
        case bgfx::RendererType::Metal:
            shaderPath = "shaders/metal/";
            break;
        case bgfx::RendererType::Nvn:
            shaderPath = "shaders/nvn/";
            break;
        case bgfx::RendererType::OpenGL:
            shaderPath = "shaders/glsl/";
            break;
        case bgfx::RendererType::OpenGLES:
            shaderPath = "shaders/essl/";
            break;
        case bgfx::RendererType::Vulkan:
            shaderPath = "shaders/spirv/";
            break;

        case bgfx::RendererType::Count:
            BX_ASSERT(false, "You should not be here!");
            break;
    }

    bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
    bx::strCat(filePath, BX_COUNTOF(filePath), _name);
    bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

    bgfx::ShaderHandle handle = bgfx::createShader(loadMem(_reader, filePath));
    bgfx::setName(handle, _name);

    return handle;
}

bgfx::ShaderHandle loadShader(const char* _name)
{
    entry::FileReader reader;
    return loadShader(&reader, _name);
}

bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName)
{
    bgfx::ShaderHandle vsh = loadShader(_reader, _vsName);
    bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
    if(NULL != _fsName)
    {
        fsh = loadShader(_reader, _fsName);
    }

    return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
}

bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName)
{
    entry::FileReader reader;
    return loadProgram(&reader, _vsName, _fsName);
}

static void imageReleaseCb(void* _ptr, void* _userData)
{
    BX_UNUSED(_ptr);
    bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
    bimg::imageFree(imageContainer);
}

bgfx::TextureHandle loadTexture(bx::FileReaderI* _reader,
                                const char* _filePath,
                                uint64_t _flags,
                                uint8_t _skip,
                                bgfx::TextureInfo* _info,
                                bimg::Orientation::Enum* _orientation)
{
    BX_UNUSED(_skip);
    bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;

    uint32_t size;
    void* data = load(_reader, entry::getAllocator(), _filePath, &size);
    if (NULL != data)
    {
        bimg::ImageContainer* imageContainer = bimg::imageParse(entry::getAllocator(), data, size);

        if (NULL != imageContainer)
        {
            if (NULL != _orientation)
            {
                *_orientation = imageContainer->m_orientation;
            }

            const bgfx::Memory* mem = bgfx::makeRef(
                imageContainer->m_data
                , imageContainer->m_size
                , imageReleaseCb
                , imageContainer
                );
            unload(data);

            if (NULL != _info)
            {
                bgfx::calcTextureSize(
                    *_info
                    , uint16_t(imageContainer->m_width)
                    , uint16_t(imageContainer->m_height)
                    , uint16_t(imageContainer->m_depth)
                    , imageContainer->m_cubeMap
                    , 1 < imageContainer->m_numMips
                    , imageContainer->m_numLayers
                    , bgfx::TextureFormat::Enum(imageContainer->m_format)
                    );
            }

            if (imageContainer->m_cubeMap)
            {
                handle = bgfx::createTextureCube(
                    uint16_t(imageContainer->m_width)
                    , 1 < imageContainer->m_numMips
                    , imageContainer->m_numLayers
                    , bgfx::TextureFormat::Enum(imageContainer->m_format)
                    , _flags
                    , mem
                    );
            }
            else if (1 < imageContainer->m_depth)
            {
                handle = bgfx::createTexture3D(
                    uint16_t(imageContainer->m_width)
                    , uint16_t(imageContainer->m_height)
                    , uint16_t(imageContainer->m_depth)
                    , 1 < imageContainer->m_numMips
                    , bgfx::TextureFormat::Enum(imageContainer->m_format)
                    , _flags
                    , mem
                    );
            }
            else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), _flags) )
            {
                handle = bgfx::createTexture2D(
                    uint16_t(imageContainer->m_width)
                    , uint16_t(imageContainer->m_height)
                    , 1 < imageContainer->m_numMips
                    , imageContainer->m_numLayers
                    , bgfx::TextureFormat::Enum(imageContainer->m_format)
                    , _flags
                    , mem
                    );
            }

            if (bgfx::isValid(handle) )
            {
                const bx::StringView name(_filePath);
                bgfx::setName(handle, name.getPtr(), name.getLength() );
            }
        }
    }

    return handle;
}

bgfx::TextureHandle loadTexture(const char* _name,
                                uint64_t _flags,
                                uint8_t _skip,
                                bgfx::TextureInfo* _info,
                                bimg::Orientation::Enum* _orientation)
{
    entry::FileReader reader;
    return loadTexture(&reader, _name, _flags, _skip, _info, _orientation);
}

bimg::ImageContainer* imageLoad(const char* _filePath, bgfx::TextureFormat::Enum _dstFormat)
{
    entry::FileReader reader;

    uint32_t size = 0;
    void* data = loadMem(&reader, entry::getAllocator(), _filePath, &size);

    return bimg::imageParse(entry::getAllocator(), data, size, bimg::TextureFormat::Enum(_dstFormat));
}

void calcTangents(void* _vertices,
                  uint16_t _numVertices,
                  bgfx::VertexLayout _layout,
                  const uint16_t* _indices,
                  uint32_t _numIndices)
{
    struct PosTexcoord
    {
        float m_x;
        float m_y;
        float m_z;
        float m_pad0;
        float m_u;
        float m_v;
        float m_pad1;
        float m_pad2;
    };

    float* tangents = new float[6 * _numVertices];
    bx::memSet(tangents, 0, 6 * _numVertices * sizeof(float));

    PosTexcoord v0;
    PosTexcoord v1;
    PosTexcoord v2;

    for(uint32_t ii = 0, num = _numIndices / 3; ii < num; ++ii)
    {
        const uint16_t* indices = &_indices[ii * 3];
        uint32_t i0 = indices[0];
        uint32_t i1 = indices[1];
        uint32_t i2 = indices[2];

        bgfx::vertexUnpack(&v0.m_x, bgfx::Attrib::Position, _layout, _vertices, i0);
        bgfx::vertexUnpack(&v0.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i0);

        bgfx::vertexUnpack(&v1.m_x, bgfx::Attrib::Position, _layout, _vertices, i1);
        bgfx::vertexUnpack(&v1.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i1);

        bgfx::vertexUnpack(&v2.m_x, bgfx::Attrib::Position, _layout, _vertices, i2);
        bgfx::vertexUnpack(&v2.m_u, bgfx::Attrib::TexCoord0, _layout, _vertices, i2);

        const float bax = v1.m_x - v0.m_x;
        const float bay = v1.m_y - v0.m_y;
        const float baz = v1.m_z - v0.m_z;
        const float bau = v1.m_u - v0.m_u;
        const float bav = v1.m_v - v0.m_v;

        const float cax = v2.m_x - v0.m_x;
        const float cay = v2.m_y - v0.m_y;
        const float caz = v2.m_z - v0.m_z;
        const float cau = v2.m_u - v0.m_u;
        const float cav = v2.m_v - v0.m_v;

        const float det = (bau * cav - bav * cau);
        const float invDet = 1.0f / det;

        const float tx = (bax * cav - cax * bav) * invDet;
        const float ty = (bay * cav - cay * bav) * invDet;
        const float tz = (baz * cav - caz * bav) * invDet;

        const float bx = (cax * bau - bax * cau) * invDet;
        const float by = (cay * bau - bay * cau) * invDet;
        const float bz = (caz * bau - baz * cau) * invDet;

        for(uint32_t jj = 0; jj < 3; ++jj)
        {
            float* tanu = &tangents[indices[jj] * 6];
            float* tanv = &tanu[3];
            tanu[0] += tx;
            tanu[1] += ty;
            tanu[2] += tz;

            tanv[0] += bx;
            tanv[1] += by;
            tanv[2] += bz;
        }
    }

    for(uint32_t ii = 0; ii < _numVertices; ++ii)
    {
        const bx::Vec3 tanu = bx::load<bx::Vec3>(&tangents[ii * 6]);
        const bx::Vec3 tanv = bx::load<bx::Vec3>(&tangents[ii * 6 + 3]);

        float nxyzw[4];
        bgfx::vertexUnpack(nxyzw, bgfx::Attrib::Normal, _layout, _vertices, ii);

        const bx::Vec3 normal = bx::load<bx::Vec3>(nxyzw);
        const float ndt = bx::dot(normal, tanu);
        const bx::Vec3 nxt = bx::cross(normal, tanu);
        const bx::Vec3 tmp = bx::sub(tanu, bx::mul(normal, ndt));

        float tangent[4];
        bx::store(tangent, bx::normalize(tmp));
        tangent[3] = bx::dot(nxt, tanv) < 0.0f ? -1.0f : 1.0f;

        bgfx::vertexPack(tangent, true, bgfx::Attrib::Tangent, _layout, _vertices, ii);
    }

    delete[] tangents;
}

bool saveToFile(bgfx::ViewId viewId, const char* _filePath, bgfx::FrameBufferHandle fbo, uint32_t width, uint32_t height)
{

    auto input_tex = bgfx::getTexture(fbo);
    // formats have one to one mapping
    auto format = bgfx::TextureFormat::RGBA8;
    auto bimg_format = static_cast<bimg::TextureFormat::Enum>(format);

    bool result = false;

    uint64_t flags = 0 | BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | BGFX_SAMPLER_U_CLAMP |
                     BGFX_SAMPLER_V_CLAMP;
    auto blit_tex = bgfx::createTexture2D(width, height, false, 1, format, flags, nullptr);

    bgfx::TextureInfo info;
    bgfx::calcTextureSize(info, width, height, 1, false, false, 1, format);

    // Blit and read
    bgfx::touch(viewId);
    bgfx::blit(viewId, blit_tex, 0, 0, input_tex);

    // Allocate memory for the texture data

    tinystl::vector<uint8_t> input(info.storageSize);

    // Read the frame buffer data
    uint32_t frameNumber = bgfx::readTexture(blit_tex, input.data());


    // Wait until the data is available (frameNumber indicates when)
    while(bgfx::frame() != frameNumber)
    {
      // You can perform other tasks here if needed
        break;
    }

    bx::FilePath filePath(_filePath);

    if(bx::makeAll(filePath.getPath()))
    {
        bx::FileWriter writer;
        if(bx::open(&writer, filePath))
        {
            bx::Error err;
            bimg::imageWritePng(&writer,
                                info.width,
                                info.height,
                                info.width * (info.bitsPerPixel / 8),
                                input.data(),
                                bimg_format,
                                false,
                                &err);
            result = true;
            bx::close(&writer);
        }
    }

    return result;
}
