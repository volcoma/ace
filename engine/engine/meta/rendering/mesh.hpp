#pragma once
#include <engine/rendering/mesh.h>
#include "texture.hpp"

#include <reflection/reflection.h>
#include <serialization/serialization.h>

namespace ace
{
REFLECT_EXTERN(mesh::info);


SAVE_EXTERN(mesh::triangle);
LOAD_EXTERN(mesh::triangle);

SAVE_EXTERN(skin_bind_data::vertex_influence);
LOAD_EXTERN(skin_bind_data::vertex_influence);

SAVE_EXTERN(skin_bind_data::bone_influence);
LOAD_EXTERN(skin_bind_data::bone_influence);

SAVE_EXTERN(skin_bind_data);
LOAD_EXTERN(skin_bind_data);

SAVE_EXTERN(mesh::armature_node);
LOAD_EXTERN(mesh::armature_node);

SAVE_EXTERN(mesh::load_data);
LOAD_EXTERN(mesh::load_data);

void save_to_file(const std::string& absolute_path, const mesh::load_data& obj);
void save_to_file_bin(const std::string& absolute_path, const mesh::load_data& obj);
void load_from_file(const std::string& absolute_path, mesh::load_data& obj);
void load_from_file_bin(const std::string& absolute_path, mesh::load_data& obj);

} // namespace ace

namespace bgfx
{
SAVE_EXTERN(VertexLayout);
LOAD_EXTERN(VertexLayout);
} // namespace bgfx
