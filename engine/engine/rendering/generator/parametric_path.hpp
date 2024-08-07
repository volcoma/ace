#ifndef GENERATOR_PARAMETRICPATH_HPP
#define GENERATOR_PARAMETRICPATH_HPP

#include <functional>

#include "edge.hpp"
#include "path_vertex.hpp"

namespace generator
{

/// Path generated by evaluating callback functions at even intervals.
class parametric_path_t
{
public:
    class edges_t
    {
    public:
        edge_t generate() const;
        bool done() const noexcept;
        void next();

    private:
        explicit edges_t(const parametric_path_t& path);

        const parametric_path_t* path_;

        int i_;

        friend class parametric_path_t;
    };

    class vertices_t
    {
    public:
        path_vertex_t generate() const;
        bool done() const noexcept;
        void next();

    private:
        explicit vertices_t(const parametric_path_t& path);

        const parametric_path_t* path_;

        int i_;

        friend class parametric_path_t;
    };

    /// @param eval A callback that should return a PathVertex for a given value.
    /// @param segments The number of segments along the path.
    /// Should be >= 1. Zero yields an empry path.
    explicit parametric_path_t(const std::function<path_vertex_t(double)>& eval, int segments = 16) noexcept;

    edges_t edges() const noexcept;

    vertices_t vertices() const noexcept;

private:
    std::function<path_vertex_t(double)> eval_;

    int segments_;

    double delta_;
};
} // namespace generator

#endif
