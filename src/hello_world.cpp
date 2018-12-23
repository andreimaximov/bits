#include <glog/logging.h>

#include <cpp/hello_world.hpp>

namespace cpp {

int hello_world() {
  LOG(INFO) << "Hello, World!";
  return 42;
}

}  // namespace cpp
