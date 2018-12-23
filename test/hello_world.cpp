#include <memory>

#include <gtest/gtest.h>

#include "cpp/hello_world.hpp"

namespace cpp {

TEST(HelloWorldTest, Returns42) {
  ASSERT_EQ(hello_world(), 42);
}

TEST(HelloWorldTest, Asan) {
  std::shared_ptr<int> p{new int{0}};
  auto raw = p.get();
  raw += 1000;
  ASSERT_DEATH(*raw = 42, "AddressSanitizer: heap-buffer-overflow on address");
}

}  // namespace cpp
