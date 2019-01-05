#include <iostream>

#include <bits/cacheline.hpp>

int main(int argc, char* argv[]) {
  auto cacheLineSize = bits::getCacheLineSize();
  if (cacheLineSize) {
    std::cout << "Cache-line: " << *cacheLineSize << " bytes" << std::endl;

  } else {
    std::cout << "Cache-line: unknown" << std::endl;
  }
  return 0;
}
