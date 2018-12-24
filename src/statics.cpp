#include <bits/statics.hpp>

namespace bits {

static char a = 'a';
thread_local static char b = 'b';

void* Statics::get() {
  return &a;
}

void* Statics::getTL() {
  return &b;
}

}  // namespace bits
