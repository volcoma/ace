#pragma once
#include "cereal_optional_nvp.h"

#define ASSOC_ARCHIVE 1

#if ASSOC_ARCHIVE == 0

#include <ser20/archives/xml.hpp>
namespace ser20
{
using oarchive_associative_t = XMLOutputArchive;
using iarchive_associative_t = XMLInputArchive;

inline auto create_oarchive_associative(std::ostream& stream)
{
    return oarchive_associative_t(stream);
}

inline auto create_iarchive_associative(std::istream& stream)
{
    return iarchive_associative_t(stream);
}
} // namespace ser20
#elif ASSOC_ARCHIVE == 1
#include <ser20/archives/simdjson.hpp>
namespace ser20
{
using oarchive_associative_t = simd::JSONOutputArchive;
using iarchive_associative_t = simd::JSONInputArchive;

inline auto create_oarchive_associative(std::ostream& stream)
{
    using options_t = oarchive_associative_t::Options;

    options_t opt(324, options_t::IndentChar::space, 1);
    return oarchive_associative_t(stream, opt);
}

inline auto create_iarchive_associative(std::istream& stream)
{
    return iarchive_associative_t(stream);
}

} // namespace ser20
#elif ASSOC_ARCHIVE == 2
#include "archives/yaml.hpp"
namespace ser20
{
using oarchive_associative_t = YAMLOutputArchive;
using iarchive_associative_t = YAMLInputArchive;

inline auto create_oarchive_associative(std::ostream& stream)
{
    return oarchive_associative_t(stream);
}

inline auto create_iarchive_associative(std::istream& stream)
{
    return iarchive_associative_t(stream);
}
} // namespace ser20
#endif
