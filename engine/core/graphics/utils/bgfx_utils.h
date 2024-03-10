/*
 * Copyright 2011-2023 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#ifndef BGFX_UTILS_H_HEADER_GUARD
#define BGFX_UTILS_H_HEADER_GUARD

#include <bgfx/bgfx.h>
#include <bimg/bimg.h>
#include <bx/bounds.h>
#include <bx/pixelformat.h>
#include <bx/string.h>

#include <tinystl/allocator.h>
#include <tinystl/vector.h>
namespace stl = tinystl;

///
void* load(const char* _filePath, uint32_t* _size = NULL);

///
void unload(void* _ptr);

///
bgfx::ShaderHandle loadShader(const char* _name);

///
bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);

///
bgfx::TextureHandle loadTexture(const char* _name,
                                uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
                                uint8_t _skip = 0,
                                bgfx::TextureInfo* _info = NULL,
                                bimg::Orientation::Enum* _orientation = NULL);

///
bimg::ImageContainer* imageLoad(const char* _filePath, bgfx::TextureFormat::Enum _dstFormat);

///
void calcTangents(void* _vertices,
                  uint16_t _numVertices,
                  bgfx::VertexLayout _layout,
                  const uint16_t* _indices,
                  uint32_t _numIndices);

/// Returns true if both internal transient index and vertex buffer have
/// enough space.
///
/// @param[in] _numVertices Number of vertices.
/// @param[in] _layout Vertex layout.
/// @param[in] _numIndices Number of indices.
///
inline bool checkAvailTransientBuffers(uint32_t _numVertices, const bgfx::VertexLayout& _layout, uint32_t _numIndices)
{
    return _numVertices == bgfx::getAvailTransientVertexBuffer(_numVertices, _layout) &&
           (0 == _numIndices || _numIndices == bgfx::getAvailTransientIndexBuffer(_numIndices));
}

///
inline uint32_t encodeNormalRgba8(float _x, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f)
{
    const float src[] = {
        _x * 0.5f + 0.5f,
        _y * 0.5f + 0.5f,
        _z * 0.5f + 0.5f,
        _w * 0.5f + 0.5f,
    };
    uint32_t dst;
    bx::packRgba8(&dst, src);
    return dst;
}

#endif // BGFX_UTILS_H_HEADER_GUARD
