#pragma once
#include "cereal_optional_nvp.h"

#define ASSOC_ARCHIVE 1

#if ASSOC_ARCHIVE == 0

#include <cereal/archives/xml.hpp>
namespace cereal
{
using oarchive_associative_t = XMLOutputArchive;
using iarchive_associative_t = XMLInputArchive;
} // namespace cereal
#elif ASSOC_ARCHIVE == 1
#include <cereal/archives/json.hpp>
namespace cereal
{
using oarchive_associative_t = JSONOutputArchive;
using iarchive_associative_t = JSONInputArchive;
} // namespace cereal
#elif ASSOC_ARCHIVE == 2
#include "archives/yaml.hpp"
namespace cereal
{
using oarchive_associative_t = YAMLOutputArchive;
using iarchive_associative_t = YAMLInputArchive;
} // namespace cereal
#endif
