#pragma once

#include <cstddef>

#include <boost/optional.hpp>

namespace bits {

// Calculates an estimate for the size of a cache-line. Assumes the cache-line
// has a power-of-2 size.
//
// Return an estimate for the size of a cache line.
boost::optional<std::size_t> getCacheLineSize();

}  // namespace bits
