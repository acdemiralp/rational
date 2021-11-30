#include "internal/doctest.h"

#include <std/experimental/rational.hpp>

TEST_CASE("std::experimental::rational")
{
  REQUIRE(std::experimental::rational( 3, 2) >  std::experimental::rational( 1, 2));
  REQUIRE(std::experimental::rational(-3, 2) <  std::experimental::rational(-1, 2));
  REQUIRE(std::experimental::rational(-3, 2) == std::experimental::rational(-6, 4));

  // TODO: More tests.
}