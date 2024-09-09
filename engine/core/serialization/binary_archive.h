#pragma once

// #include "ser20/archives/portable_binary.hpp"
//  namespace ser20
//{
//     using oarchive_binary_t = PortableBinaryOutputArchive;
//     using iarchive_binary_t = PortableBinaryInputArchive;
// }

#include "ser20/archives/binary.hpp"
namespace ser20
{
using oarchive_binary_t = BinaryOutputArchive;
using iarchive_binary_t = BinaryInputArchive;
} // namespace ser20
