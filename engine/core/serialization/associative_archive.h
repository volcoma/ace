#pragma once
#include "cereal_optional_nvp.h"

#define ASSOC_ARCHIVE 1

#if ASSOC_ARCHIVE == 0

#include <ser20/archives/xml.hpp>
namespace ser20
{
using oarchive_associative_t = XMLOutputArchive;
using iarchive_associative_t = XMLInputArchive;
} // namespace ser20
#elif ASSOC_ARCHIVE == 1
#include <ser20/archives/json.hpp>
namespace ser20
{
using oarchive_associative_t = JSONOutputArchive;
using iarchive_associative_t = JSONInputArchive;
} // namespace ser20
#elif ASSOC_ARCHIVE == 2
#include "archives/yaml.hpp"
namespace ser20
{
using oarchive_associative_t = YAMLOutputArchive;
using iarchive_associative_t = YAMLInputArchive;
} // namespace ser20
#endif
