#ifndef GENERATOR_UTILS_HPP
#define GENERATOR_UTILS_HPP
#include <utility>
namespace generator
{

/// Will have a type named "Type" that has same type as value returned by method
/// generate() of type Generator.
template<typename generator_t>
class generated_type
{
public:
    using type = decltype(std::declval<const generator_t>().generate());
};

/// Will have a type named "Type" that has same type as value returned by method
/// edges() for type Primitive.
template<typename primitive_t>
class edge_generator_type
{
public:
    using type = decltype(std::declval<const primitive_t>().edges());
};

/// Will have a type named "Type" that has same type as value returned by method
/// triangles() for type Primitive.
template<typename primitive_t>
class triangle_generator_type
{
public:
    using type = decltype(std::declval<const primitive_t>().triangles());
};

/// Will have a type named "Type" that has same type as value returned by method
/// vertices() for type Primitive.
template<typename primitive_t>
class vertex_generator_type
{
public:
    using type = decltype(std::declval<const primitive_t>().vertices());
};

/// Counts the number of steps left in the generator.
template<typename generator_t>
int count(const generator_t& generator) noexcept
{
    generator_t temp{generator};
    int c = 0;
    while(!temp.done())
    {
        ++c;
        temp.next();
    }
    return c;
}
} // namespace generator

#endif
