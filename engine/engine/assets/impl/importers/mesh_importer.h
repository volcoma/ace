#pragma once
#include <engine/animation/animation.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/material.h>

namespace ace
{
namespace importer
{
struct imported_material
{
    std::string name;
    std::shared_ptr<material> material;
};

bool load_mesh_data_from_file(const std::string& path, mesh::load_data& load_data,
                              std::vector<animation>& animations,
                              std::vector<imported_material>& materials);
}
}
