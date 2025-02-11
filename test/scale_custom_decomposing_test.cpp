/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::impl::memory::decode;
using scale::impl::memory::encode;

struct CustomDecomposableObject {
  CustomDecomposableObject() : a(0xff), b(0xff), c(0xff), d(0xff), e(0xff) {}
  CustomDecomposableObject(
      uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e)
      : a(a), b(b), c(c), d(d), e(e) {};

  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;

  bool operator==(const CustomDecomposableObject &other) const = default;

  SCALE_CUSTOM_DECOMPOSITION(CustomDecomposableObject, b, c, d);
};

TEST(CustomDecomposable, encode) {
  CustomDecomposableObject x(1, 2, 3, 4, 5);

  std::vector expected = {x.b, x.c, x.d};

  ASSERT_OUTCOME_SUCCESS(actual, encode(x));
  EXPECT_EQ(expected, actual);
}

TEST(CustomDecomposable, decode) {
  std::vector<uint8_t> data = {1, 2, 3};

  CustomDecomposableObject expected(0xff, 1, 2, 3, 0xff);

  ASSERT_OUTCOME_SUCCESS(actual, decode<CustomDecomposableObject>(data));
  EXPECT_EQ(expected, actual);
}
