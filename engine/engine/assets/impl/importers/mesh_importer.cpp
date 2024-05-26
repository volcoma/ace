#include "mesh_importer.h"

#include <graphics/graphics.h>
#include <logging/logging.h>
#include <math/math.h>
#include <string_utils/utils.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/LogStream.hpp>
#include <assimp/ProgressHandler.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <graphics/utils/bgfx_utils.h>

#include <algorithm>
#include <filesystem/filesystem.h>

namespace ace
{
namespace importer
{
namespace
{
math::transform process_matrix(const aiMatrix4x4& assimp_matrix)
{
    math::mat4 matrix;

    matrix[0][0] = assimp_matrix.a1;
    matrix[1][0] = assimp_matrix.a2;
    matrix[2][0] = assimp_matrix.a3;
    matrix[3][0] = assimp_matrix.a4;

    matrix[0][1] = assimp_matrix.b1;
    matrix[1][1] = assimp_matrix.b2;
    matrix[2][1] = assimp_matrix.b3;
    matrix[3][1] = assimp_matrix.b4;

    matrix[0][2] = assimp_matrix.c1;
    matrix[1][2] = assimp_matrix.c2;
    matrix[2][2] = assimp_matrix.c3;
    matrix[3][2] = assimp_matrix.c4;

    matrix[0][3] = assimp_matrix.d1;
    matrix[1][3] = assimp_matrix.d2;
    matrix[2][3] = assimp_matrix.d3;
    matrix[3][3] = assimp_matrix.d4;

    return matrix;
}

void process_vertices(aiMesh* mesh, mesh::load_data& load_data)
{
    // Determine the correct offset to any relevant elements in the vertex
    bool has_position = load_data.vertex_format.has(gfx::attribute::Position);
    bool has_normal = load_data.vertex_format.has(gfx::attribute::Normal);
    bool has_bitangent = load_data.vertex_format.has(gfx::attribute::Bitangent);
    bool has_tangent = load_data.vertex_format.has(gfx::attribute::Tangent);
    bool has_texcoord0 = load_data.vertex_format.has(gfx::attribute::TexCoord0);
    auto vertex_stride = load_data.vertex_format.getStride();

    std::uint32_t current_vertex = load_data.vertex_count;
    load_data.vertex_count += mesh->mNumVertices;
    load_data.vertex_data.resize(load_data.vertex_count * vertex_stride);

    std::uint8_t* current_vertex_ptr = &load_data.vertex_data[0] + current_vertex * vertex_stride;

    for(size_t i = 0; i < mesh->mNumVertices; ++i, current_vertex_ptr += vertex_stride)
    {
        // position
        if(mesh->mVertices != nullptr && has_position)
        {
            float position[4];
            std::memcpy(position, &mesh->mVertices[i], sizeof(math::vec3));

            if(has_position)
            {
                gfx::vertex_pack(position,
                                 false,
                                 gfx::attribute::Position,
                                 load_data.vertex_format,
                                 current_vertex_ptr);
            }
        }

        // tex coords
        if(mesh->mTextureCoords[0] != nullptr && has_texcoord0)
        {
            float textureCoords[4];
            std::memcpy(textureCoords, &mesh->mTextureCoords[0][i], sizeof(math::vec2));

            if(has_texcoord0)
            {
                gfx::vertex_pack(textureCoords,
                                 true,
                                 gfx::attribute::TexCoord0,
                                 load_data.vertex_format,
                                 current_vertex_ptr);
            }
        }

        ////normals
        math::vec4 normal;
        if(mesh->mNormals != nullptr && has_normal)
        {
            std::memcpy(math::value_ptr(normal), &mesh->mNormals[i], sizeof(math::vec3));

            if(has_normal)
            {
                gfx::vertex_pack(math::value_ptr(normal),
                                 true,
                                 gfx::attribute::Normal,
                                 load_data.vertex_format,
                                 current_vertex_ptr);
            }
        }

        math::vec4 tangent;
        // tangents
        if(mesh->mTangents != nullptr && has_tangent)
        {
            std::memcpy(math::value_ptr(tangent), &mesh->mTangents[i], sizeof(math::vec3));
            tangent.w = 1.0f;
            if(has_tangent)
            {
                gfx::vertex_pack(math::value_ptr(tangent),
                                 true,
                                 gfx::attribute::Tangent,
                                 load_data.vertex_format,
                                 current_vertex_ptr);
            }
        }

        // binormals
        math::vec4 bitangent;
        if(mesh->mBitangents != nullptr && has_bitangent)
        {
            std::memcpy(math::value_ptr(bitangent), &mesh->mBitangents[i], sizeof(math::vec3));
            float handedness =
                math::dot(math::vec3(bitangent), math::normalize(math::cross(math::vec3(normal), math::vec3(tangent))));
            tangent.w = handedness;

            if(has_bitangent)
            {
                gfx::vertex_pack(math::value_ptr(bitangent),
                                 true,
                                 gfx::attribute::Bitangent,
                                 load_data.vertex_format,
                                 current_vertex_ptr);
            }
        }
    }
}

void process_faces(aiMesh* mesh, std::uint32_t subset_offset, mesh::load_data& load_data)
{
    load_data.triangle_count += mesh->mNumFaces;

    for(size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];

        mesh::triangle triangle;

        triangle.data_group_id = mesh->mMaterialIndex;
        load_data.material_count = std::max(load_data.material_count, triangle.data_group_id + 1);

        auto num_indices = std::min<size_t>(face.mNumIndices, 3);

        for(size_t j = 0; j < num_indices; ++j)
        {
            triangle.indices[j] = face.mIndices[j] + subset_offset;
        }

        load_data.triangle_data.push_back(triangle);
    }
}

void process_bones(aiMesh* mesh, std::uint32_t subset_offset, mesh::load_data& load_data)
{
    if(mesh->mBones != nullptr)
    {
        auto& bone_influences = load_data.skin_data.get_bones();

        for(size_t i = 0; i < mesh->mNumBones; ++i)
        {
            aiBone* assimp_bone = mesh->mBones[i];
            const std::string bone_name = assimp_bone->mName.C_Str();

            auto it = std::find_if(std::begin(bone_influences),
                                   std::end(bone_influences),
                                   [&bone_name](const auto& bone)
                                   {
                                       return bone_name == bone.bone_id;
                                   });

            skin_bind_data::bone_influence* bone_ptr = nullptr;
            if(it != std::end(bone_influences))
            {
                bone_ptr = &(*it);
            }
            else
            {
                const auto& assimp_matrix = assimp_bone->mOffsetMatrix;
                skin_bind_data::bone_influence bone_influence;
                bone_influence.bone_id = bone_name;
                bone_influence.bind_pose_transform = process_matrix(assimp_matrix);
                bone_influences.emplace_back(std::move(bone_influence));
                bone_ptr = &bone_influences.back();
            }

            if(bone_ptr == nullptr)
            {
                continue;
            }

            for(size_t j = 0; j < assimp_bone->mNumWeights; ++j)
            {
                aiVertexWeight assimp_influence = assimp_bone->mWeights[j];

                skin_bind_data::vertex_influence influence;
                influence.vertex_index = assimp_influence.mVertexId + subset_offset;
                influence.weight = assimp_influence.mWeight;

                bone_ptr->influences.emplace_back(influence);
            }
        }
    }
}

void process_mesh(aiMesh* mesh, mesh::load_data& load_data)
{
    const auto mesh_vertices_offset = load_data.vertex_count;
    process_faces(mesh, mesh_vertices_offset, load_data);
    process_bones(mesh, mesh_vertices_offset, load_data);
    process_vertices(mesh, load_data);
}

void process_meshes(const aiScene* scene, mesh::load_data& load_data)
{
    for(size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[i];
        process_mesh(mesh, load_data);
    }
}

void process_node(const aiNode* node, std::unique_ptr<mesh::armature_node>& armature_node)
{
    armature_node->mesh_count = node->mNumMeshes;
    armature_node->children.resize(node->mNumChildren);
    armature_node->name = node->mName.C_Str();
    armature_node->local_transform = process_matrix(node->mTransformation);
    for(size_t i = 0; i < node->mNumChildren; ++i)
    {
        armature_node->children[i] = std::make_unique<mesh::armature_node>();
        process_node(node->mChildren[i], armature_node->children[i]);
    }
}

void process_nodes(const aiScene* scene, mesh::load_data& load_data)
{
    if(scene->mRootNode != nullptr)
    {
        load_data.root_node = std::make_unique<mesh::armature_node>();
        process_node(scene->mRootNode, load_data.root_node);

        auto get_axis = [&](const std::string& name, math::vec3 fallback)
        {
            int axis = 0;
            if(!scene->mMetaData->Get<int>(name, axis))
            {
                return fallback;
            }
            int axisSign = 1;
            if(!scene->mMetaData->Get<int>(name + "Sign", axisSign))
            {
                return fallback;
            }
            math::vec3 result{0.0f, 0.0f, 0.0f};

            if(axis < 0 || axis >= 3)
            {
                return fallback;
            }

            result[axis] = axisSign;

            return result;
        };
        auto x_axis = get_axis("CoordAxis", {1.0f, 0.0f, 0.0f});
        auto y_axis = get_axis("UpAxis", {0.0f, 1.0f, 0.0f});
        auto z_axis = get_axis("FrontAxis", {0.0f, 0.0f, 1.0f});
        load_data.root_node->local_transform.set_rotation(x_axis, y_axis, z_axis);
    }
}

void process_animation(const aiAnimation* assimp_anim, animation& anim)
{
    anim.name = assimp_anim->mName.C_Str();
    auto ticks_per_second = assimp_anim->mTicksPerSecond;
    if(ticks_per_second < 0.001)
    {
        ticks_per_second = 25.0;
    }

    auto ticks = assimp_anim->mDuration;

    anim.duration = decltype(anim.duration)(ticks * ticks_per_second);

    if(assimp_anim->mNumChannels > 0)
    {
        anim.channels.resize(assimp_anim->mNumChannels);
    }

    for(size_t i = 0; i < assimp_anim->mNumChannels; ++i)
    {
        const aiNodeAnim* assimp_node_anim = assimp_anim->mChannels[i];
        auto& node_anim = anim.channels[i];
        node_anim.node_name = assimp_node_anim->mNodeName.C_Str();

        if(assimp_node_anim->mNumPositionKeys > 0)
        {
            node_anim.position_keys.resize(assimp_node_anim->mNumPositionKeys);
        }

        for(size_t idx = 0; idx < assimp_node_anim->mNumPositionKeys; ++idx)
        {
            const auto& anim_key = assimp_node_anim->mPositionKeys[idx];
            auto& key = node_anim.position_keys[idx];
            key.time = decltype(key.time)(anim_key.mTime);
            key.value.x = anim_key.mValue.x;
            key.value.y = anim_key.mValue.y;
            key.value.z = anim_key.mValue.z;
        }

        if(assimp_node_anim->mNumRotationKeys > 0)
        {
            node_anim.rotation_keys.resize(assimp_node_anim->mNumRotationKeys);
        }

        for(size_t idx = 0; idx < assimp_node_anim->mNumRotationKeys; ++idx)
        {
            const auto& anim_key = assimp_node_anim->mRotationKeys[idx];
            auto& key = node_anim.rotation_keys[idx];
            key.time = decltype(key.time)(anim_key.mTime);
            key.value.x = anim_key.mValue.x;
            key.value.y = anim_key.mValue.y;
            key.value.z = anim_key.mValue.z;
            key.value.w = anim_key.mValue.w;
        }

        if(assimp_node_anim->mNumScalingKeys > 0)
        {
            node_anim.scaling_keys.resize(assimp_node_anim->mNumScalingKeys);
        }

        for(size_t idx = 0; idx < assimp_node_anim->mNumScalingKeys; ++idx)
        {
            const auto& anim_key = assimp_node_anim->mScalingKeys[idx];
            auto& key = node_anim.scaling_keys[idx];
            key.time = decltype(key.time)(anim_key.mTime);
            key.value.x = anim_key.mValue.x;
            key.value.y = anim_key.mValue.y;
            key.value.z = anim_key.mValue.z;
        }
    }
}
void process_animations(const aiScene* scene, std::vector<animation>& animations)
{
    if(scene->mNumAnimations > 0)
    {
        animations.resize(scene->mNumAnimations);
    }

    for(size_t i = 0; i < scene->mNumAnimations; ++i)
    {
        const aiAnimation* assimp_anim = scene->mAnimations[i];
        auto& anim = animations[i];
        process_animation(assimp_anim, anim);
    }
}

void process_material(asset_manager& am,
                      const fs::path& output_dir,
                      const aiMaterial* material,
                      pbr_material& mat,
                      std::vector<imported_texture>& textures)
{
    // technically there is a difference between MASK and BLEND mode
    // but for our purposes it's enough if we sort properly
    aiString alphaMode;
    material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    aiString alphaModeOpaque;
    alphaModeOpaque.Set("OPAQUE");

    // out.blend = alphaMode != alphaModeOpaque;

    bool doubleSided{};
    material->Get(AI_MATKEY_TWOSIDED, doubleSided);

    // texture files
    aiString fileBaseColor, fileMetallic, fileRoughness, fileNormals, fileOcclusion, fileEmissive;
    material->GetTexture(AI_MATKEY_BASE_COLOR_TEXTURE, &fileBaseColor);
    material->GetTexture(AI_MATKEY_METALLIC_TEXTURE, &fileMetallic);
    material->GetTexture(AI_MATKEY_ROUGHNESS_TEXTURE, &fileRoughness);

    material->GetTexture(aiTextureType_NORMALS, 0, &fileNormals);

    aiTextureType emissiveType = aiTextureType_EMISSION_COLOR;
    material->GetTexture(emissiveType, 0, &fileEmissive);

    if(fileEmissive.length == 0)
    {
        emissiveType = aiTextureType_EMISSIVE;
        material->GetTexture(emissiveType, 0, &fileEmissive);
    }

    aiTextureType occlusionType = aiTextureType_AMBIENT_OCCLUSION;
    material->GetTexture(occlusionType, 0, &fileOcclusion);
    if(fileOcclusion.length == 0)
    {
        occlusionType = aiTextureType_AMBIENT;
        material->GetTexture(occlusionType, 0, &fileOcclusion);
    }

    if(fileOcclusion.length == 0)
    {
        occlusionType = aiTextureType_LIGHTMAP;
        material->GetTexture(occlusionType, 0, &fileOcclusion);
    }

    // TODO AI_MATKEY_METALLIC_TEXTURE + AI_MATKEY_ROUGHNESS_TEXTURE
    aiString fileMetallicRoughness;
    material->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &fileMetallicRoughness);

    for(uint32_t i = 0; i < material->mNumProperties; i++)
    {
        auto prop = material->mProperties[i];

        APPLOG_INFO("Material Property:");
        APPLOG_INFO("  Name = {0}", prop->mKey.C_Str());
        // APPLOG_INFO("  Type = {0}", prop->mType);

        if(prop->mDataLength > 0 && prop->mData)
        {
            APPLOG_INFO("  Size = {0}", prop->mDataLength);

            switch(prop->mType)
            {
                case aiPropertyTypeInfo::aiPTI_Float:
                {
                    float data = *(float*)prop->mData;
                    APPLOG_INFO("  Value = {0}", data);
                    break;
                }

                case aiPropertyTypeInfo::aiPTI_Double:
                {
                    double data = *(double*)prop->mData;
                    APPLOG_INFO("  Value = {0}", data);
                    break;
                }

                case aiPropertyTypeInfo::aiPTI_String:
                {
                    aiString data = *(aiString*)prop->mData;
                    APPLOG_INFO("  Value = {0}", data.C_Str());
                    break;
                }

                case aiPropertyTypeInfo::aiPTI_Integer:
                {
                    int32_t data = *(int32_t*)prop->mData;
                    APPLOG_INFO("  Value = {0}", data);

                    break;
                }

                case aiPropertyTypeInfo::aiPTI_Buffer:
                {
                    fmt::string_view view(prop->mData, prop->mDataLength);
                    APPLOG_INFO("  Value = {0}", view);
                    break;
                }
                default:
                    break;
            }
        }
        APPLOG_INFO("  Semantic = {0}", aiTextureTypeToString(aiTextureType(prop->mSemantic)));
    }

    // diffuse
    if(fileBaseColor.length > 0)
    {
        textures.emplace_back(imported_texture{fileBaseColor.C_Str()});

        auto key = fs::convert_to_protocol(output_dir / fileBaseColor.C_Str());
        mat.set_color_map(am.get_asset<gfx::texture>(key.generic_string()));
    }

    aiColor4D baseColorFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_BASE_COLOR, baseColorFactor))
    {
        math::color base_color{};
        base_color = {baseColorFactor.r, baseColorFactor.g, baseColorFactor.b, baseColorFactor.a};
        base_color = glm::clamp(base_color.value, 0.0f, 1.0f);
        mat.set_base_color(base_color);
    }

    // metallic/roughness
    if(fileMetallic.length > 0)
    {
        textures.emplace_back(imported_texture{fileMetallic.C_Str()});

        auto key = fs::convert_to_protocol(output_dir / fileMetallic.C_Str());
        mat.set_metalness_map(am.get_asset<gfx::texture>(key.generic_string()));
    }

    ai_real metallicFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor))
    {
        mat.set_metalness(glm::clamp(metallicFactor, 0.0f, 1.0f));
    }
    else
    {

        if(AI_SUCCESS == material->Get(AI_MATKEY_REFLECTIVITY, metallicFactor))
        {
            mat.set_metalness(glm::clamp(metallicFactor, 0.0f, 1.0f));
        }

    }

    if(fileRoughness.length > 0)
    {
        textures.emplace_back(imported_texture{fileRoughness.C_Str()});

        auto key = fs::convert_to_protocol(output_dir / fileRoughness.C_Str());
        mat.set_roughness_map(am.get_asset<gfx::texture>(key.generic_string()));
    }

    ai_real roughnessFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor))
    {
        mat.set_roughness(glm::clamp(roughnessFactor, 0.0f, 1.0f));
    }
    else
    {
        ai_real glossinessFactor;
        if(AI_SUCCESS == material->Get(AI_MATKEY_GLOSSINESS_FACTOR, glossinessFactor))
        {
            mat.set_roughness(1.0f - glm::clamp(glossinessFactor, 0.0f, 1.0f));
        }
    }

    // normal map
    if(fileNormals.length > 0)
    {
        textures.emplace_back(imported_texture{fileNormals.C_Str()});

        auto key = fs::convert_to_protocol(output_dir / fileNormals.C_Str());
        mat.set_normal_map(am.get_asset<gfx::texture>(key.generic_string()));
    }

    ai_real normalScale;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), normalScale))
    {
        mat.set_bumpiness(normalScale);
    }

    // occlusion texture
    if(fileOcclusion.length > 0)
    {
        textures.emplace_back(imported_texture{fileOcclusion.C_Str()});

        auto key = fs::convert_to_protocol(output_dir / fileOcclusion.C_Str());
        mat.set_ao_map(am.get_asset<gfx::texture>(key.generic_string()));
    }

    ai_real occlusionStrength;
    if(AI_SUCCESS == material->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(occlusionType, 0), occlusionStrength))
    {
        // out.occlusionStrength = glm::clamp(occlusionStrength, 0.0f, 1.0f);
    }

    // emissive texture
    if(fileEmissive.length > 0)
    {
        textures.emplace_back(imported_texture{fileEmissive.C_Str()});

        auto key = fs::convert_to_protocol(output_dir / fileEmissive.C_Str());
        mat.set_emissive_map(am.get_asset<gfx::texture>(key.generic_string()));
    }

    aiColor3D emissiveFactor;
    if(AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor))
    {
        math::color emissive{};
        emissive = {emissiveFactor.r, emissiveFactor.g, emissiveFactor.b};
        emissive = glm::clamp(emissive.value, 0.0f, 1.0f);
        mat.set_emissive_color(emissive);
    }
}

void process_materials(asset_manager& am,
                       const fs::path& output_dir,
                       const aiScene* scene,
                       std::vector<imported_material>& materials,
                       std::vector<imported_texture>& textures)
{
    if(scene->mNumMaterials > 0)
    {
        materials.resize(scene->mNumMaterials);
    }

    for(size_t i = 0; i < scene->mNumMaterials; ++i)
    {
        const aiMaterial* assimp_mat = scene->mMaterials[i];

        auto mat = std::make_shared<pbr_material>();
        process_material(am, output_dir, assimp_mat, *mat, textures);
        materials[i].mat = mat;
        materials[i].name = string_utils::replace(fmt::format("{}_{}", i, assimp_mat->GetName().C_Str()), ".", "_");
    }
}

void process_textures(asset_manager& am,
                      const fs::path& output_dir,
                      const aiScene* scene,
                      std::vector<imported_texture>& textures)
{
    if(scene->mNumTextures > 0)
    {
        textures.resize(textures.size() + scene->mNumTextures);
    }

    for(size_t i = 0; i < scene->mNumTextures; ++i)
    {
        const aiTexture* assimp_tex = scene->mTextures[i];

        if(assimp_tex->mFilename.length > 0)
        {
            textures[i].name = fs::path(assimp_tex->mFilename.C_Str()).filename().string();
        }

        if(assimp_tex->mHeight == 0 && assimp_tex->pcData)
        {
        }
    }
}

void process_imported_scene(asset_manager& am,
                            const fs::path& output_dir,
                            const aiScene* scene,
                            mesh::load_data& load_data,
                            std::vector<animation>& animations,
                            std::vector<imported_material>& materials,
                            std::vector<imported_texture>& textures)
{
    load_data.vertex_format = gfx::mesh_vertex::get_layout();
    process_materials(am, output_dir, scene, materials, textures);
    process_textures(am, output_dir, scene, textures);
    process_meshes(scene, load_data);
    process_nodes(scene, load_data);
    process_animations(scene, animations);
}
} // namespace

bool load_mesh_data_from_file(asset_manager& am,
                              const fs::path& path,
                              const fs::path& output_dir,
                              mesh::load_data& load_data,
                              std::vector<animation>& animations,
                              std::vector<imported_material>& materials,
                              std::vector<imported_texture>& textures)
{
    struct LogStream : public Assimp::LogStream
    {
        static void Initialize()
        {
            if(Assimp::DefaultLogger::isNullLogger())
            {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new LogStream, Assimp::Logger::Err | Assimp::Logger::Warn);
            }
        }

        virtual void write(const char* message) override
        {
            APPLOG_WARNING("Mesh Importer: {0}", message);
        }
    };

    LogStream::Initialize();

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);

    if(path.extension() == "fbx")
    {
        importer.SetPropertyBool(AI_CONFIG_FBX_CONVERT_TO_M, true);
        importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);
    }

    unsigned int flags = aiProcess_ConvertToLeftHanded |
                         aiProcessPreset_TargetRealtime_Quality | // some optimizations and safety checks
                         aiProcess_OptimizeMeshes |               // minimize number of meshes
                         aiProcess_OptimizeGraph | aiProcess_TransformUVCoords | aiProcess_GlobalScale;

    const aiScene* scene = importer.ReadFile(path.string(), flags);

    if(scene == nullptr)
    {
        APPLOG_ERROR(importer.GetErrorString());
        return false;
    }

    process_imported_scene(am, output_dir, scene, load_data, animations, materials, textures);

    return true;
}
} // namespace importer

} // namespace ace
