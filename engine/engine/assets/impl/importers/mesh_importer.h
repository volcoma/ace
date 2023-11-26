#pragma once
#include <engine/animation/animation.h>
#include <engine/rendering/mesh.h>

namespace ace
{
namespace importer
{
bool load_mesh_data_from_file(const std::string& path, mesh::load_data& load_data,
							  std::vector<animation>& animations);
}
}
