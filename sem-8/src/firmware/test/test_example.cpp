#include <gtest/gtest.h>

#include "sensing/example.hpp"

TEST(Example, AddsTwoNumbers) {
  EXPECT_EQ(sensing::add(2, 3), 5);
}
