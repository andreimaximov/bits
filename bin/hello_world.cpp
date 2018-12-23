#include <glog/logging.h>

#include "cpp/hello_world.hpp"

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  cpp::hello_world();
  return 0;
}
