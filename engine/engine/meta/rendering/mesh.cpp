#include "mesh.hpp"

#include <engine/meta/core/math/quaternion.hpp>
#include <engine/meta/core/math/transform.hpp>
#include <engine/meta/core/math/bbox.hpp>

#include <fstream>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

#include <serialization/types/array.hpp>

namespace bgfx
{
SAVE(VertexLayout)
{
    try_save(ar, ser20::make_nvp("hash", obj.m_hash));
    try_save(ar, ser20::make_nvp("stride", obj.m_stride));
    try_save(ar, ser20::make_nvp("offset", obj.m_offset));
    try_save(ar, ser20::make_nvp("attributes", obj.m_attributes));
}
SAVE_INSTANTIATE(VertexLayout, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(VertexLayout, ser20::oarchive_associative_t);

LOAD(VertexLayout)
{
    try_load(ar, ser20::make_nvp("hash", obj.m_hash));
    try_load(ar, ser20::make_nvp("stride", obj.m_stride));
    try_load(ar, ser20::make_nvp("offset", obj.m_offset));
    try_load(ar, ser20::make_nvp("attributes", obj.m_attributes));
}
LOAD_INSTANTIATE(VertexLayout, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(VertexLayout, ser20::oarchive_associative_t);

} // namespace bgfx

namespace ace
{
using mesh_info_t = mesh::info;
REFLECT(mesh_info_t)
{
    rttr::registration::class_<mesh::info>("info")
        .property_readonly("vertices", &mesh::info::vertices)(rttr::metadata("pretty_name", "Vertices"),
                                                              rttr::metadata("tooltip", "Vertices count."))
        .property_readonly("primitives", &mesh::info::primitives)(rttr::metadata("pretty_name", "Primitives"),
                                                                  rttr::metadata("tooltip", "Primitives count."))
        .property_readonly("submeshes", &mesh::info::submeshes)(rttr::metadata("pretty_name", "submeshes"),
                                                            rttr::metadata("tooltip", "submeshes count."))
        .property_readonly("data_groups", &mesh::info::data_groups)(rttr::metadata("pretty_name", "Material Groups"),
                                                            rttr::metadata("tooltip", "Materials count."));
}

SAVE(mesh::submesh)
{
    try_save(ar, ser20::make_nvp("data_group_id", obj.data_group_id));
    try_save(ar, ser20::make_nvp("vertex_start", obj.vertex_start));
    try_save(ar, ser20::make_nvp("vertex_count", obj.vertex_count));
    try_save(ar, ser20::make_nvp("face_start", obj.face_start));
    try_save(ar, ser20::make_nvp("face_count", obj.face_count));
    try_save(ar, ser20::make_nvp("bbox", obj.bbox));
    try_save(ar, ser20::make_nvp("node_id", obj.node_id));
    try_save(ar, ser20::make_nvp("skinned", obj.skinned));

}
SAVE_INSTANTIATE(mesh::submesh, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(mesh::submesh, ser20::oarchive_associative_t);

LOAD(mesh::submesh)
{
    try_load(ar, ser20::make_nvp("data_group_id", obj.data_group_id));
    try_load(ar, ser20::make_nvp("vertex_start", obj.vertex_start));
    try_load(ar, ser20::make_nvp("vertex_count", obj.vertex_count));
    try_load(ar, ser20::make_nvp("face_start", obj.face_start));
    try_load(ar, ser20::make_nvp("face_count", obj.face_count));
    try_load(ar, ser20::make_nvp("bbox", obj.bbox));
    try_load(ar, ser20::make_nvp("node_id", obj.node_id));
    try_load(ar, ser20::make_nvp("skinned", obj.skinned));


}
LOAD_INSTANTIATE(mesh::submesh, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(mesh::submesh, ser20::iarchive_associative_t);

SAVE(mesh::triangle)
{
    try_save(ar, ser20::make_nvp("data_group_id", obj.data_group_id));
    try_save(ar, ser20::make_nvp("indices", obj.indices));
    try_save(ar, ser20::make_nvp("flags", obj.flags));
}
SAVE_INSTANTIATE(mesh::triangle, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(mesh::triangle, ser20::oarchive_associative_t);

LOAD(mesh::triangle)
{
    try_load(ar, ser20::make_nvp("data_group_id", obj.data_group_id));
    try_load(ar, ser20::make_nvp("indices", obj.indices));
    try_load(ar, ser20::make_nvp("flags", obj.flags));
}
LOAD_INSTANTIATE(mesh::triangle, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(mesh::triangle, ser20::iarchive_associative_t);

SAVE(skin_bind_data::vertex_influence)
{
    try_save(ar, ser20::make_nvp("vertex_index", obj.vertex_index));
    try_save(ar, ser20::make_nvp("weight", obj.weight));
}
SAVE_INSTANTIATE(skin_bind_data::vertex_influence, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(skin_bind_data::vertex_influence, ser20::oarchive_associative_t);

LOAD(skin_bind_data::vertex_influence)
{
    try_load(ar, ser20::make_nvp("vertex_index", obj.vertex_index));
    try_load(ar, ser20::make_nvp("weight", obj.weight));
}
LOAD_INSTANTIATE(skin_bind_data::vertex_influence, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(skin_bind_data::vertex_influence, ser20::iarchive_associative_t);

SAVE(skin_bind_data::bone_influence)
{
    try_save(ar, ser20::make_nvp("bone_id", obj.bone_id));
    try_save(ar, ser20::make_nvp("bind_pose_transform", obj.bind_pose_transform));
    try_save(ar, ser20::make_nvp("influences", obj.influences));
}
SAVE_INSTANTIATE(skin_bind_data::bone_influence, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(skin_bind_data::bone_influence, ser20::oarchive_associative_t);

LOAD(skin_bind_data::bone_influence)
{
    try_load(ar, ser20::make_nvp("bone_id", obj.bone_id));
    try_load(ar, ser20::make_nvp("bind_pose_transform", obj.bind_pose_transform));
    try_load(ar, ser20::make_nvp("influences", obj.influences));
}
LOAD_INSTANTIATE(skin_bind_data::bone_influence, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(skin_bind_data::bone_influence, ser20::iarchive_associative_t);

SAVE(skin_bind_data)
{
    try_save(ar, ser20::make_nvp("bones", obj.get_bones()));
}
SAVE_INSTANTIATE(skin_bind_data, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(skin_bind_data, ser20::oarchive_associative_t);

LOAD(skin_bind_data)
{
    try_load(ar, ser20::make_nvp("bones", obj.get_bones()));
}
LOAD_INSTANTIATE(skin_bind_data, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(skin_bind_data, ser20::iarchive_associative_t);

SAVE(mesh::armature_node)
{
    try_save(ar, ser20::make_nvp("name", obj.name));
    try_save(ar, ser20::make_nvp("local_transform", obj.local_transform));
    try_save(ar, ser20::make_nvp("children", obj.children));
    try_save(ar, ser20::make_nvp("submeshes", obj.submeshes));

}
SAVE_INSTANTIATE(mesh::armature_node, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(mesh::armature_node, ser20::oarchive_associative_t);

LOAD(mesh::armature_node)
{
    try_load(ar, ser20::make_nvp("name", obj.name));
    try_load(ar, ser20::make_nvp("local_transform", obj.local_transform));
    try_load(ar, ser20::make_nvp("children", obj.children));
    try_load(ar, ser20::make_nvp("submeshes", obj.submeshes));
}
LOAD_INSTANTIATE(mesh::armature_node, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(mesh::armature_node, ser20::iarchive_associative_t);

SAVE(mesh::load_data)
{
    try_save(ar, ser20::make_nvp("vertex_format", obj.vertex_format));
    try_save(ar, ser20::make_nvp("vertex_count", obj.vertex_count));
    try_save(ar, ser20::make_nvp("vertex_data", obj.vertex_data));
    try_save(ar, ser20::make_nvp("triangle_count", obj.triangle_count));
    try_save(ar, ser20::make_nvp("triangle_data", obj.triangle_data));
    try_save(ar, ser20::make_nvp("material_count", obj.material_count));
    try_save(ar, ser20::make_nvp("submeshes", obj.submeshes));
    try_save(ar, ser20::make_nvp("skin_data", obj.skin_data));
    try_save(ar, ser20::make_nvp("root_node", obj.root_node));
    try_save(ar, ser20::make_nvp("bbox", obj.bbox));

}
SAVE_INSTANTIATE(mesh::load_data, ser20::oarchive_binary_t);
SAVE_INSTANTIATE(mesh::load_data, ser20::oarchive_associative_t);

LOAD(mesh::load_data)
{
    try_load(ar, ser20::make_nvp("vertex_format", obj.vertex_format));
    try_load(ar, ser20::make_nvp("vertex_count", obj.vertex_count));
    try_load(ar, ser20::make_nvp("vertex_data", obj.vertex_data));
    try_load(ar, ser20::make_nvp("triangle_count", obj.triangle_count));
    try_load(ar, ser20::make_nvp("triangle_data", obj.triangle_data));
    try_load(ar, ser20::make_nvp("material_count", obj.material_count));
    try_load(ar, ser20::make_nvp("submeshes", obj.submeshes));
    try_load(ar, ser20::make_nvp("skin_data", obj.skin_data));
    try_load(ar, ser20::make_nvp("root_node", obj.root_node));
    try_load(ar, ser20::make_nvp("bbox", obj.bbox));

}
LOAD_INSTANTIATE(mesh::load_data, ser20::iarchive_binary_t);
LOAD_INSTANTIATE(mesh::load_data, ser20::iarchive_associative_t);

void save_to_file(const std::string& absolute_path, const mesh::load_data& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        ser20::oarchive_associative_t ar(stream);
        try_save(ar, ser20::make_nvp("mesh", obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const mesh::load_data& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("mesh", obj));
    }
}

void load_from_file(const std::string& absolute_path, mesh::load_data& obj)
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        ser20::iarchive_associative_t ar(stream);
        try_load(ar, ser20::make_nvp("mesh", obj));
    }
}

void load_from_file_bin(const std::string& absolute_path, mesh::load_data& obj)
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::iarchive_binary_t ar(stream);
        try_load(ar, ser20::make_nvp("mesh", obj));
    }
}

} // namespace ace
