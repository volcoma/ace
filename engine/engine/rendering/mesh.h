#pragma once

#include <base/basetypes.hpp>
#include <graphics/graphics.h>
#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

#include <map>
#include <memory>
#include <vector>

namespace ace
{
class camera;

namespace triangle_flags
{
enum e
{
    none = 0,
    degenerate = 0x1,
};
} // namespace triangle_flags

enum class mesh_status
{
    not_prepared,
    preparing,
    prepared
};

enum class mesh_create_origin
{
    bottom,
    center,
    top
};

/**
 * @brief Structure describing how a skinned mesh should be bound to any bones
 * that influence its vertices.
 */
class skin_bind_data
{
    REFLECTABLE(skin_bind_data)
    SERIALIZABLE(skin_bind_data)
public:
    /**
     * @brief Describes how a bone influences a specific vertex.
     */
    struct vertex_influence
    {
        std::uint32_t vertex_index = 0; ///< The index of the vertex influenced by the bone.
        float weight = 0.0f;            ///< The weight of the influence.

        vertex_influence() = default;
        vertex_influence(std::uint32_t _index, float _weight) : vertex_index(_index), weight(_weight)
        {
        }
    };
    using vertex_influence_array_t = std::vector<vertex_influence>;

    /**
     * @brief Describes the vertices that are connected to the referenced bone and how much influence it has on them.
     */
    struct bone_influence
    {
        std::string bone_id;                 ///< The unique identifier of the bone.
        math::transform bind_pose_transform; ///< The "bind pose" or "offset" transform of the bone.
        vertex_influence_array_t influences; ///< List of vertices influenced by the bone.
    };
    using bone_influence_array_t = std::vector<bone_influence>;

    /**
     * @brief Contains per-vertex influence and weight information.
     */
    struct vertex_data
    {
        std::vector<std::int32_t> influences; ///< List of bones that influence this vertex.
        std::vector<float> weights;           ///< List of weights for each influence.
        std::int32_t palette;                 ///< Index of the palette to which this vertex has been assigned.
        std::uint32_t original_vertex;        ///< The index of the original vertex.
    };
    using vertex_data_array_t = std::vector<vertex_data>;

    /**
     * @brief Adds influence information for a specific bone.
     *
     * @param bone The bone influence information to add.
     */
    void add_bone(const bone_influence& bone);

    /**
     * @brief Removes any bones that do not contain any influences.
     */
    void remove_empty_bones();

    /**
     * @brief Constructs a list of bone influences and weights for each vertex based on the binding data provided.
     *
     * @param vertex_count The total number of vertices.
     * @param vertex_remap The remap array for vertices.
     * @param table The output table of vertex data.
     */
    void build_vertex_table(std::uint32_t vertex_count,
                            const std::vector<std::uint32_t>& vertex_remap,
                            vertex_data_array_t& table);

    /**
     * @brief Remaps the vertex references stored in the binding based on the supplied remap array.
     *
     * @param remap The remap array.
     */
    void remap_vertices(const std::vector<std::uint32_t>& remap);

    /**
     * @brief Retrieves a list of all bones that influence the skin in some way.
     *
     * @return const bone_influence_array_t& The list of bone influences.
     */
    const bone_influence_array_t& get_bones() const;

    /**
     * @brief Retrieves a list of all bones that influence the skin in some way.
     *
     * @return bone_influence_array_t& The list of bone influences.
     */
    bone_influence_array_t& get_bones();

    /**
     * @brief Checks whether the skin data has any bones.
     *
     * @return true If there are bones.
     * @return false If there are no bones.
     */
    bool has_bones() const;

    /**
     * @brief Finds a bone by its unique identifier.
     *
     * @param id The unique identifier of the bone.
     * @return const bone_influence* Pointer to the bone influence data if found, otherwise nullptr.
     */
    const bone_influence* find_bone_by_id(const std::string& id) const;

    /**
     * @brief Releases memory allocated for vertex influences in each stored bone.
     */
    void clear_vertex_influences();

    /**
     * @brief Clears out the bone information stored in this object.
     */
    void clear();

private:
    bone_influence_array_t bones_; ///< List of bones that influence the skin mesh vertices.
};

/**
 * @brief Outlines a collection of bones that influence a given set of faces/vertices in the mesh.
 */
class bone_palette
{
public:
    using bone_index_map_t = std::map<std::uint32_t, std::uint32_t>;

    /**
     * @brief Constructs a bone palette with the given size.
     *
     * @param paletteSize The maximum size of the palette.
     */
    bone_palette(std::uint32_t paletteSize);

    /**
     * @brief Copy constructor.
     *
     * @param init The bone palette to copy from.
     */
    bone_palette(const bone_palette& init);

    /**
     * @brief Destructor.
     */
    ~bone_palette();

    /**
     * @brief Gathers the bone/palette information and matrices ready for drawing the skinned mesh.
     *
     * @param node_transforms The node transforms.
     * @param bind_data The skin bind data.
     * @param compute_inverse_transpose Whether to compute the inverse transpose of the matrices.
     * @return std::vector<math::transform> The skinning matrices.
     */
    std::vector<math::transform> get_skinning_matrices(const std::vector<math::transform>& node_transforms,
                                                       const skin_bind_data& bind_data,
                                                       bool compute_inverse_transpose) const;

    /**
     * @brief Determines the relevant "fit" information that can be used to discover if and how the specified
     * combination of bones will fit into this palette.
     *
     * @param input The input bone index map.
     * @param current_space The current space available in the palette.
     * @param common_base The common base index.
     * @param additional_bones The number of additional bones required.
     */
    void compute_palette_fit(bone_index_map_t& input,
                             std::int32_t& current_space,
                             std::int32_t& common_base,
                             std::int32_t& additional_bones);

    /**
     * @brief Assigns the specified bones (and faces) to this bone palette.
     *
     * @param bones The bones to assign.
     * @param faces The faces influenced by these bones.
     */
    void assign_bones(bone_index_map_t& bones, std::vector<std::uint32_t>& faces);

    /**
     * @brief Assigns the specified bones to this bone palette.
     *
     * @param bones The bones to assign.
     */
    void assign_bones(const std::vector<std::uint32_t>& bones);

    /**
     * @brief Translates the specified bone index into its associated position in the palette.
     *
     * @param bone_index The bone index.
     * @return std::uint32_t The position in the palette, or -1 if not found.
     */
    std::uint32_t translate_bone_to_palette(std::uint32_t bone_index) const;

    /**
     * @brief Retrieves the maximum vertex blend index for this palette.
     *
     * @return std::int32_t The maximum vertex blend index.
     */
    std::int32_t get_maximum_blend_index() const;

    /**
     * @brief Retrieves the maximum size of the palette.
     *
     * @return std::uint32_t The maximum size of the palette.
     */
    std::uint32_t get_maximum_size() const;

    /**
     * @brief Retrieves the identifier of the data group assigned to the subset of the mesh reserved for this bone
     * palette.
     *
     * @return std::uint32_t The data group identifier.
     */
    std::uint32_t get_data_group() const;

    /**
     * @brief Retrieves the list of faces assigned to this palette.
     *
     * @return std::vector<std::uint32_t>& The list of faces.
     */
    std::vector<std::uint32_t>& get_influenced_faces();

    /**
     * @brief Retrieves the indices of the bones referenced by this palette.
     *
     * @return const std::vector<std::uint32_t>& The list of bone indices.
     */
    const std::vector<std::uint32_t>& get_bones() const;

    /**
     * @brief Sets the maximum vertex blend index for this palette.
     *
     * @param index The maximum vertex blend index.
     */
    void set_maximum_blend_index(int index);

    /**
     * @brief Sets the identifier of the data group assigned to the subset of the mesh reserved for this bone palette.
     *
     * @param group The data group identifier.
     */
    void set_data_group(std::uint32_t group);

    /**
     * @brief Clears out the temporary face influences array.
     */
    void clear_influenced_faces();

protected:
    bone_index_map_t bones_lut_; ///< Sorted list of bones in this palette.
    std::vector<std::uint32_t>
        bones_; ///< Main palette of indices that reference the bones outlined in the main skin binding data.
    std::vector<std::uint32_t> faces_; ///< List of faces assigned to this palette.
    std::uint32_t data_group_id_; ///< The data group identifier used to separate the mesh data into subsets relevant to
                                  ///< this bone palette.
    std::uint32_t maximum_size_;  ///< The maximum size of the palette.
    std::int32_t maximum_blend_index_; ///< The maximum vertex blend index for this palette.
};

/**
 * @brief Main class representing a 3D mesh with support for different LODs, subsets, and skinning.
 */
class mesh
{
public:
    /**
     * @brief Structure describing an individual "piece" of the mesh, often grouped by material, but can be any
     * arbitrary collection of triangles.
     */
    struct subset
    {
        std::uint32_t data_group_id =
            0;                          ///< The unique user assigned "data group" that can be used to separate subsets.
        std::int32_t vertex_start = -1; ///< The beginning vertex for this batch.
        std::uint32_t vertex_count = 0; ///< Number of vertices included in this batch.
        std::int32_t face_start = -1;   ///< The initial face, from the index buffer, to render in this batch.
        std::uint32_t face_count = 0;   ///< Number of faces to render in this batch.
    };

    struct info
    {
        std::uint32_t vertices = 0;   ///< Total number of vertices.
        std::uint32_t primitives = 0; ///< Total number of primitives.
        std::uint32_t subsets = 0;    ///< Total number of subsets.
    };

    /**
     * @brief Structure describing data for a single triangle in the mesh.
     */
    struct triangle
    {
        std::uint32_t data_group_id = 0;      ///< Data group identifier for this triangle.
        std::uint32_t indices[3] = {0, 0, 0}; ///< Indices of the vertices in this triangle.
        std::uint8_t flags = 0;               ///< Flags for this triangle.
    };

    using triangle_array_t = std::vector<triangle>;
    using subset_array_t = std::vector<subset*>;
    using bone_palette_array_t = std::vector<bone_palette>;

    struct armature_node
    {
        uint32_t mesh_count{};                                ///< Count of meshes in this armature node.
        std::string name;                                     ///< Name of the armature node.
        math::transform local_transform;                      ///< Local transform of the armature node.
        std::vector<std::unique_ptr<armature_node>> children; ///< Children nodes of this armature node.
    };

    /**
     * @brief Struct used for mesh construction.
     */
    struct load_data
    {
        gfx::vertex_layout vertex_format;                   ///< The format of the vertex data.
        std::vector<std::uint8_t> vertex_data;              ///< Vertex data buffer.
        std::uint32_t vertex_count = 0;                     ///< Total number of vertices.
        triangle_array_t triangle_data;                     ///< Triangle data buffer.
        std::uint32_t triangle_count = 0;                   ///< Total number of triangles.
        std::uint32_t material_count = 0;                   ///< Total number of materials.
        skin_bind_data skin_data;                           ///< Skin data for this mesh.
        std::unique_ptr<armature_node> root_node = nullptr; ///< Root node of the armature.
    };

    /**
     * @brief Constructs a mesh object.
     */
    mesh();

    /**
     * @brief Destructor.
     */
    ~mesh();

    /**
     * @brief Clears out all the mesh data.
     */
    void dispose();

    /**
     * @brief Binds the render buffers for rendering the entire mesh.
     */
    void bind_render_buffers();

    /**
     * @brief Binds the render buffers for rendering a specific subset of the mesh.
     *
     * @param data_group_id The data group identifier of the subset to render.
     */
    void bind_render_buffers_for_subset(std::uint32_t data_group_id);

    /**
     * @brief Prepares the mesh with the specified vertex format.
     *
     * @param vertex_format The vertex format to use.
     * @return true If the mesh was successfully prepared.
     * @return false If the mesh preparation failed.
     */
    bool prepare_mesh(const gfx::vertex_layout& vertex_format);

    /**
     * @brief Prepares the mesh with the specified data.
     *
     * @param vertex_format The vertex format to use.
     * @param vertices The vertex data.
     * @param vertex_count The number of vertices.
     * @param faces The triangle data.
     * @param hardware_copy Whether to use hardware copy.
     * @param weld Whether to weld vertices.
     * @param optimize Whether to optimize the mesh.
     * @return true If the mesh was successfully prepared.
     * @return false If the mesh preparation failed.
     */
    bool prepare_mesh(const gfx::vertex_layout& vertex_format,
                      void* vertices,
                      std::uint32_t vertex_count,
                      const triangle_array_t& faces,
                      bool hardware_copy = true,
                      bool weld = true,
                      bool optimize = true);

    /**
     * @brief Sets the source of the vertex buffer to pull data from while preparing the mesh.
     *
     * @param source The source vertex data.
     * @param vertex_count The number of vertices.
     * @param source_format The format of the source vertex data.
     * @return true If the vertex source was successfully set.
     * @return false If setting the vertex source failed.
     */
    bool set_vertex_source(void* source, std::uint32_t vertex_count, const gfx::vertex_layout& source_format);

    /**
     * @brief Adds primitives (triangles) to the mesh.
     *
     * @param triangles The triangles to add.
     * @return true If the primitives were successfully added.
     * @return false If adding the primitives failed.
     */
    bool add_primitives(const triangle_array_t& triangles);

    /**
     * @brief Binds the mesh as a skin with the specified skin binding data.
     *
     * @param bind_data The skin binding data.
     * @return true If the mesh was successfully bound as a skin.
     * @return false If binding the mesh as a skin failed.
     */
    bool bind_skin(const skin_bind_data& bind_data);

    /**
     * @brief Binds the armature tree.
     *
     * @param root The root node of the armature.
     * @return true If the armature was successfully bound.
     * @return false If binding the armature failed.
     */
    bool bind_armature(std::unique_ptr<armature_node>& root);

    /**
     * @brief Sets the number of subsets for the mesh.
     *
     * @param count The number of subsets.
     */
    void set_subset_count(std::uint32_t count);

    /**
     * @brief Creates a plane geometry.
     *
     * @param format The vertex format.
     * @param width The width of the plane.
     * @param height The height of the plane.
     * @param width_segments The number of segments along the width.
     * @param height_segments The number of segments along the height.
     * @param origin The origin of the plane.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the plane was successfully created.
     * @return false If creating the plane failed.
     */
    bool create_plane(const gfx::vertex_layout& format,
                      float width,
                      float height,
                      std::uint32_t width_segments,
                      std::uint32_t height_segments,
                      mesh_create_origin origin,
                      bool hardware_copy = true);

    /**
     * @brief Creates a cube geometry.
     *
     * @param format The vertex format.
     * @param width The width of the cube.
     * @param height The height of the cube.
     * @param depth The depth of the cube.
     * @param width_segments The number of segments along the width.
     * @param height_segments The number of segments along the height.
     * @param depth_segments The number of segments along the depth.
     * @param origin The origin of the cube.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the cube was successfully created.
     * @return false If creating the cube failed.
     */
    bool create_cube(const gfx::vertex_layout& format,
                     float width,
                     float height,
                     float depth,
                     std::uint32_t width_segments,
                     std::uint32_t height_segments,
                     std::uint32_t depth_segments,
                     mesh_create_origin origin,
                     bool hardware_copy = true);

    /**
     * @brief Creates a sphere geometry.
     *
     * @param format The vertex format.
     * @param radius The radius of the sphere.
     * @param stacks The number of stacks.
     * @param slices The number of slices.
     * @param origin The origin of the sphere.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the sphere was successfully created.
     * @return false If creating the sphere failed.
     */
    bool create_sphere(const gfx::vertex_layout& format,
                       float radius,
                       std::uint32_t stacks,
                       std::uint32_t slices,
                       mesh_create_origin origin,
                       bool hardware_copy = true);

    /**
     * @brief Creates a cylinder geometry.
     *
     * @param format The vertex format.
     * @param radius The radius of the cylinder.
     * @param height The height of the cylinder.
     * @param stacks The number of stacks.
     * @param slices The number of slices.
     * @param origin The origin of the cylinder.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the cylinder was successfully created.
     * @return false If creating the cylinder failed.
     */
    bool create_cylinder(const gfx::vertex_layout& format,
                         float radius,
                         float height,
                         std::uint32_t stacks,
                         std::uint32_t slices,
                         mesh_create_origin origin,
                         bool hardware_copy = true);

    /**
     * @brief Creates a capsule geometry.
     *
     * @param format The vertex format.
     * @param radius The radius of the capsule.
     * @param height The height of the capsule.
     * @param stacks The number of stacks.
     * @param slices The number of slices.
     * @param origin The origin of the capsule.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the capsule was successfully created.
     * @return false If creating the capsule failed.
     */
    bool create_capsule(const gfx::vertex_layout& format,
                        float radius,
                        float height,
                        std::uint32_t stacks,
                        std::uint32_t slices,
                        mesh_create_origin origin,
                        bool hardware_copy = true);

    /**
     * @brief Creates a cone geometry.
     *
     * @param format The vertex format.
     * @param radius The base radius of the cone.
     * @param radiusTip The tip radius of the cone.
     * @param height The height of the cone.
     * @param stacks The number of stacks.
     * @param slices The number of slices.
     * @param origin The origin of the cone.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the cone was successfully created.
     * @return false If creating the cone failed.
     */
    bool create_cone(const gfx::vertex_layout& format,
                     float radius,
                     float radiusTip,
                     float height,
                     std::uint32_t stacks,
                     std::uint32_t slices,
                     mesh_create_origin origin,
                     bool hardware_copy = true);

    /**
     * @brief Creates a torus geometry.
     *
     * @param format The vertex format.
     * @param outer_radius The outer radius of the torus.
     * @param inner_radius The inner radius of the torus.
     * @param bands The number of bands.
     * @param sides The number of sides.
     * @param origin The origin of the torus.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the torus was successfully created.
     * @return false If creating the torus failed.
     */
    bool create_torus(const gfx::vertex_layout& format,
                      float outer_radius,
                      float inner_radius,
                      std::uint32_t bands,
                      std::uint32_t sides,
                      mesh_create_origin origin,
                      bool hardware_copy = true);

    /**
     * @brief Creates a teapot geometry.
     *
     * @param format The vertex format.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the teapot was successfully created.
     * @return false If creating the teapot failed.
     */
    bool create_teapot(const gfx::vertex_layout& format, bool hardware_copy = true);

    /**
     * @brief Creates an icosahedron geometry.
     *
     * @param format The vertex format.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the icosahedron was successfully created.
     * @return false If creating the icosahedron failed.
     */
    bool create_icosahedron(const gfx::vertex_layout& format, bool hardware_copy = true);

    /**
     * @brief Creates a dodecahedron geometry.
     *
     * @param format The vertex format.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the dodecahedron was successfully created.
     * @return false If creating the dodecahedron failed.
     */
    bool create_dodecahedron(const gfx::vertex_layout& format, bool hardware_copy = true);

    /**
     * @brief Creates an icosphere geometry.
     *
     * @param format The vertex format.
     * @param tesselation_level The level of tesselation.
     * @param hardware_copy Whether to use hardware copy.
     * @return true If the icosphere was successfully created.
     * @return false If creating the icosphere failed.
     */
    bool create_icosphere(const gfx::vertex_layout& format, int tesselation_level, bool hardware_copy = true);

    /**
     * @brief Ends the preparation of the mesh and builds the render data.
     *
     * @param hardware_copy Whether to use hardware copy.
     * @param weld Whether to weld vertices.
     * @param optimize Whether to optimize the mesh.
     * @param build_buffers Whether to build the render buffers.
     * @return true If the mesh was successfully prepared.
     * @return false If the mesh preparation failed.
     */
    bool end_prepare(bool hardware_copy = true, bool weld = true, bool optimize = true, bool build_buffers = true);

    /**
     * @brief Builds the internal vertex buffer.
     *
     * @param hardware_copy Whether to use hardware copy.
     */
    void build_vb(bool hardware_copy = true);

    /**
     * @brief Builds the internal index buffer.
     *
     * @param hardware_copy Whether to use hardware copy.
     */
    void build_ib(bool hardware_copy = true);

    /**
     * @brief Generates edge-triangle adjacency information for the mesh data.
     *
     * @param adjacency The adjacency information output array.
     * @return true If the adjacency information was successfully generated.
     * @return false If generating the adjacency information failed.
     */
    bool generate_adjacency(std::vector<std::uint32_t>& adjacency);

    /**
     * @brief Determines the number of faces stored in the mesh.
     *
     * @return std::uint32_t The number of faces.
     */
    std::uint32_t get_face_count() const;

    /**
     * @brief Determines the number of vertices stored in the mesh.
     *
     * @return std::uint32_t The number of vertices.
     */
    std::uint32_t get_vertex_count() const;

    /**
     * @brief Retrieves the underlying vertex data from the mesh.
     *
     * @return std::uint8_t* The vertex data.
     */
    std::uint8_t* get_system_vb();

    /**
     * @brief Retrieves the underlying index data from the mesh.
     *
     * @return std::uint32_t* The index data.
     */
    std::uint32_t* get_system_ib();

    /**
     * @brief Retrieves the format of the underlying mesh vertex data.
     *
     * @return const gfx::vertex_layout& The vertex format.
     */
    const gfx::vertex_layout& get_vertex_format() const;

    /**
     * @brief Retrieves the skin bind data if this mesh has been bound as a skin.
     *
     * @return const skin_bind_data& The skin bind data.
     */
    const skin_bind_data& get_skin_bind_data() const;

    /**
     * @brief Retrieves the compiled bone combination palette data if this mesh has been bound as a skin.
     *
     * @return const bone_palette_array_t& The bone palette data.
     */
    const bone_palette_array_t& get_bone_palettes() const;

    /**
     * @brief Retrieves the armature tree of the mesh.
     *
     * @return const std::unique_ptr<armature_node>& The root node of the armature tree.
     */
    const std::unique_ptr<armature_node>& get_armature() const;

    /**
     * @brief Calculates the screen rectangle of the mesh based on its world transform and the camera.
     *
     * @param world The world transform of the mesh.
     * @param cam The camera.
     * @return irect32_t The screen rectangle.
     */
    irect32_t calculate_screen_rect(const math::transform& world, const camera& cam) const;

    /**
     * @brief Retrieves information about the subset of the mesh associated with the specified data group identifier.
     *
     * @param data_group_id The data group identifier.
     * @return const subset* Pointer to the subset information.
     */
    const subset* get_subset(std::uint32_t data_group_id = 0) const;

    /**
     * @brief Gets the local bounding box for this mesh.
     *
     * @return const math::bbox& The bounding box.
     */
    inline const math::bbox& get_bounds() const
    {
        return bbox_;
    }

    /**
     * @brief Gets the preparation status for this mesh.
     *
     * @return mesh_status The preparation status.
     */
    inline mesh_status get_status() const
    {
        return prepare_status_;
    }

    /**
     * @brief Gets the number of subsets for this mesh.
     *
     * @return std::size_t The number of subsets.
     */
    inline std::size_t get_subset_count() const
    {
        return mesh_subsets_.size();
    }

    using data_group_subset_map_t = std::map<std::uint32_t, subset_array_t>;
    using byte_array_t = std::vector<std::uint8_t>;
    struct preparation_data
    {
        enum flags
        {
            source_contains_normal = 0x1,
            source_contains_binormal = 0x2,
            source_contains_tangent = 0x4
        };

        std::uint8_t* vertex_source = nullptr; ///< The source vertex data currently being used to prepare the mesh.
        bool owns_source = false;              ///< Whether the source data is owned by this object.
        gfx::vertex_layout source_format; ///< The format of the vertex data currently being used to prepare the mesh.
        std::vector<std::uint32_t> vertex_records; ///< Records the location in the vertex buffer that each vertex has
                                                   ///< been placed during data insertion.
        byte_array_t vertex_data;                  ///< Final vertex buffer currently being prepared.
        byte_array_t vertex_flags;                 ///< Additional descriptive information about the vertices.
        triangle_array_t triangle_data;            ///< Stores the current face/triangle data.
        std::uint32_t triangle_count = 0;          ///< Total number of triangles currently stored.
        std::uint32_t vertex_count = 0;            ///< Total number of vertices currently stored.
        bool compute_normals = false;              ///< Whether to compute vertex normals.
        bool compute_binormals = false;            ///< Whether to compute vertex binormals.
        bool compute_tangents = false;             ///< Whether to compute vertex tangents.
        bool compute_barycentric = false;          ///< Whether to compute vertex barycentric coordinates.
    };

protected:
    struct optimizer_vertex_info
    {
        std::int32_t cache_position = -1; ///< The position of the vertex in the pseudo-cache.
        float vertex_score = 0.0f;        ///< The score associated with this vertex.
        std::uint32_t unused_triangle_references =
            0; ///< Total number of triangles that reference this vertex that have not yet been added.
        std::vector<std::uint32_t> triangle_references; ///< List of all triangles referencing this vertex.
    };

    struct optimizer_triangle_info
    {
        float triangle_score = 0.0f; ///< The sum of all three child vertex scores.
        bool added = false;          ///< Whether the triangle has been added to the draw list.
    };

    struct adjacent_edge_key
    {
        const math::vec3* vertex1 = nullptr; ///< Pointer to the first vertex in the edge.
        const math::vec3* vertex2 = nullptr; ///< Pointer to the second vertex in the edge.
    };

    struct mesh_subset_key
    {
        std::uint32_t data_group_id = 0; ///< The data group identifier for this subset.

        mesh_subset_key() = default;
        mesh_subset_key(std::uint32_t _dataGroupId) : data_group_id(_dataGroupId)
        {
        }
    };

    using subset_key_map_t = std::map<mesh_subset_key, subset*>;
    using subset_key_array_t = std::vector<mesh_subset_key>;

    struct weld_key
    {
        std::uint8_t* vertex;      ///< Pointer to the vertex.
        gfx::vertex_layout format; ///< Format of the vertex.
        float tolerance;           ///< Tolerance for welding vertices.
    };

    struct face_influences
    {
        bone_palette::bone_index_map_t bones; ///< List of unique bones that influence a given number of faces.
    };

    struct bone_combination_key
    {
        face_influences* influences = nullptr; ///< Pointer to the face influences.
        std::uint32_t data_group_id = 0;       ///< The data group identifier.

        bone_combination_key(face_influences* _influences, std::uint32_t group_id)
            : influences(_influences)
            , data_group_id(group_id)
        {
        }
    };

    using bone_combination_map_t = std::map<bone_combination_key, std::vector<std::uint32_t>*>;

    friend bool operator<(const adjacent_edge_key& key1, const adjacent_edge_key& key2);
    friend bool operator<(const mesh_subset_key& key1, const mesh_subset_key& key2);
    friend bool operator<(const weld_key& key1, const weld_key& key2);
    friend bool operator<(const bone_combination_key& key1, const bone_combination_key& key2);

    /**
     * @brief Generates any vertex components that may be missing, such as normals, tangents, or binormals.
     *
     * @param weld Whether to weld vertices.
     * @return true If the vertex components were successfully generated.
     * @return false If generating the vertex components failed.
     */
    bool generate_vertex_components(bool weld);

    /**
     * @brief Generates vertex normals for the mesh.
     *
     * @param adjacency_ptr Pointer to the adjacency information.
     * @param remap_array_ptr Pointer to the vertex remap array.
     * @return true If the vertex normals were successfully generated.
     * @return false If generating the vertex normals failed.
     */
    bool generate_vertex_normals(std::uint32_t* adjacency_ptr, std::vector<std::uint32_t>* remap_array_ptr = nullptr);

    /**
     * @brief Generates vertex barycentric coordinates for the mesh.
     *
     * @param adjacency Pointer to the adjacency information.
     * @return true If the vertex barycentric coordinates were successfully generated.
     * @return false If generating the vertex barycentric coordinates failed.
     */
    bool generate_vertex_barycentrics(std::uint32_t* adjacency);

    /**
     * @brief Generates vertex tangents for the mesh.
     *
     * @return true If the vertex tangents were successfully generated.
     * @return false If generating the vertex tangents failed.
     */
    bool generate_vertex_tangents();

    /**
     * @brief Welds the vertices together that can be combined.
     *
     * @param tolerance The tolerance for welding vertices.
     * @param vertexRemap Pointer to the vertex remap array.
     * @return true If the vertices were successfully welded.
     * @return false If welding the vertices failed.
     */
    bool weld_vertices(float tolerance = 0.000001f, std::vector<std::uint32_t>* vertexRemap = nullptr);

    /**
     * @brief Sorts the data in the mesh into material and data group order.
     *
     * @param optimize Whether to optimize the mesh.
     * @param hardware_copy Whether to use hardware copy.
     * @param build_buffer Whether to build the render buffer.
     * @return true If the mesh data was successfully sorted.
     * @return false If sorting the mesh data failed.
     */
    bool sort_mesh_data(bool optimize, bool hardware_copy, bool build_buffer);

    /**
     * @brief Binds the mesh data for rendering the selected batch of primitives.
     *
     * @param face_start The starting face index.
     * @param face_count The number of faces to render.
     * @param vertex_start The starting vertex index.
     * @param vertex_count The number of vertices to render.
     */
    void bind_mesh_data(std::uint32_t face_start,
                        std::uint32_t face_count,
                        std::uint32_t vertex_start,
                        std::uint32_t vertex_count);

    /**
     * @brief Calculates the best order for triangle data, optimizing for efficient use of the hardware vertex cache.
     *
     * @param subset The subset to optimize.
     * @param source_buffer_ptr Pointer to the source index buffer.
     * @param destination_buffer_ptr Pointer to the destination index buffer.
     * @param minimum_vertex The minimum vertex index.
     * @param maximum_vertex The maximum vertex index.
     */
    static void build_optimized_index_buffer(const subset* subset,
                                             std::uint32_t* source_buffer_ptr,
                                             std::uint32_t* destination_buffer_ptr,
                                             std::uint32_t minimum_vertex,
                                             std::uint32_t maximum_vertex);

    /**
     * @brief Generates scores used to identify important vertices when ordering triangle data.
     *
     * @param vertex_info_ptr Pointer to the vertex information.
     * @return float The vertex score.
     */
    static float find_vertex_optimizer_score(const optimizer_vertex_info* vertex_info_ptr);

protected:
    bool force_tangent_generation_ = false;     ///< Whether to force the generation of tangent space vectors.
    bool force_normal_generation_ = false;      ///< Whether to force the generation of vertex normals.
    bool force_barycentric_generation_ = false; ///< Whether to force the generation of vertex barycentric coordinates.
    bool disable_final_sort_ = false;           ///< Whether to disable the automatic re-sort operation.

    std::uint8_t* system_vb_ = nullptr;  ///< The vertex data during data insertion and system memory copy.
    gfx::vertex_layout vertex_format_;   ///< The vertex format used for the mesh internal vertex data.
    std::uint32_t* system_ib_ = nullptr; ///< The final system memory copy of the index buffer.
    subset_key_array_t triangle_data_;   ///< Material and data group information for each triangle.
    std::shared_ptr<void> hardware_vb_;  ///< The actual hardware vertex buffer resource.
    std::shared_ptr<void> hardware_ib_;  ///< The actual hardware index buffer resource.

    subset_array_t mesh_subsets_;         ///< The actual list of subsets maintained by this mesh.
    data_group_subset_map_t data_groups_; ///< Lookup information mapping data groups to subsets batched by material.
    subset_key_map_t subset_lookup_;      ///< Quick lookup of existing subsets based on material and data group ID.

    bool hardware_mesh_ = true;      ///< Whether the mesh uses a hardware vertex/index buffer.
    bool optimize_mesh_ = false;     ///< Whether the mesh was optimized when it was prepared.
    math::bbox bbox_;                ///< Axis aligned bounding box describing object dimensions in object space.
    std::uint32_t face_count_ = 0;   ///< Total number of faces in the prepared mesh.
    std::uint32_t vertex_count_ = 0; ///< Total number of vertices in the prepared mesh.

    mesh_status prepare_status_ = mesh_status::not_prepared; ///< Preparation status of the mesh.
    preparation_data preparation_data_;                      ///< Input data used for constructing the final mesh.

    skin_bind_data
        skin_bind_data_; ///< Data describing how the mesh should be bound as a skin with supplied bone matrices.
    bone_palette_array_t bone_palettes_;            ///< List of unique combinations of bones to use during rendering.
    std::unique_ptr<armature_node> root_ = nullptr; ///< List of armature nodes.
};

} // namespace ace
