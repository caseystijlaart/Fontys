#include <iostream>

#include "sensing/example.hpp"

int main() {
  std::cout << "sensing_app toolchain smoke test: 2 + 3 = " << sensing::add(2, 3) << '\n';
  return 0;
}
