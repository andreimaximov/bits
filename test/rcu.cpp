#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <bits/rcu.hpp>

namespace bits {

class RcuTest : public ::testing::Test {
 public:
  void TearDown() override {
    if (thread.joinable()) thread.join();
  }

  template <typename F>
  void RunInThread(F&& f) {
    done = false;
    thread = std::thread{[this, f{std::move(f)}] {
      f();
      done = true;
    }};
  }

  static void SleepMs(std::uint64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds{ms});
  }

  bool done = false;
  std::thread thread;
};

TEST_F(RcuTest, Sync) {
  RunInThread([this]() {
    RcuSnapshot<> snap;
    SleepMs(200);
    done = true;
  });

  SleepMs(100);
  RcuSnapshot<>::sync();

  ASSERT_TRUE(done);
}

TEST_F(RcuTest, MoveCons) {
  RunInThread([this]() {
    RcuSnapshot<> snap1;
    RcuSnapshot<> snap2{std::move(snap1)};
    SleepMs(200);
    done = true;
  });

  SleepMs(100);
  RcuSnapshot<>::sync();

  ASSERT_TRUE(done);
}

TEST_F(RcuTest, MoveOp) {
  RunInThread([this]() {
    RcuSnapshot<> snap1;
    RcuSnapshot<> snap2;
    snap1 = std::move(snap2);
    SleepMs(200);
    done = true;
  });

  SleepMs(100);
  RcuSnapshot<>::sync();

  ASSERT_TRUE(done);
}

}  // namespace bits
