#include <glog/logging.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
