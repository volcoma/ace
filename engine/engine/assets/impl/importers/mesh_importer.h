#pragma once
#include <engine/animation/animation.h>
#include <engine/assets/asset_manager.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>

namespace ace
{
namespace importer
{
struct imported_material
{
    std::string name;
    std::shared_ptr<material> mat;
};

struct imported_texture
{
    std::string name;
    std::string semantic;
    bool inverse{};
    int embedded_index{-1};
    int process_count{};
};

bool load_mesh_data_from_file(asset_manager& am,
                              const fs::path& path,
                              mesh::load_data& load_data,
                              std::vector<animation>& animations,
                              std::vector<imported_material>& materials,
                              std::vector<imported_texture>& textures);
} // namespace importer
} // namespace ace
