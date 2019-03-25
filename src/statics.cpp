#include <bits/statics.hpp>

namespace bits {
namespace {

char a = 'a';
thread_local char b = 'b';

}  // namespace

void* Statics::get() {
  return &a;
}

void* Statics::getTL() {
  return &b;
}

}  // namespace bits
