#include <gtest/gtest.h>

#include <bits/cacheline.hpp>

namespace bits {

TEST(CacheLineTest, GetCacheLineSize) {
  auto cacheLineSize = getCacheLineSize();
  ASSERT_TRUE(cacheLineSize);
  ASSERT_GT(*cacheLineSize, 0);
}

}  // namespace bits
