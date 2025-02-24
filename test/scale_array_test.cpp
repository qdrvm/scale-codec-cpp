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

/**
 * @given collection of N of type uint8_t
 * @when encode array and decode back
 * @then given equal array
 */
template <size_t N, typename Array = std::array<uint8_t, N>>
void testArray() {
  for (auto value : {0b0000'0000,
                     0b0011'0011,
                     0b0101'0101,
                     0b1010'1010,
                     0b1100'1100,
                     0b1111'1111}) {
    Array testee{};
    std::ranges::fill(testee, value);

    ASSERT_OUTCOME_SUCCESS(data, encode(testee));
    ASSERT_OUTCOME_SUCCESS(result, decode<Array>(data));

    EXPECT_EQ(testee, result);
  }
}

/// Doing test of encode/decode different size arrays
TEST(Scale, encodeArray) {
  testArray<0>();
  testArray<127>();
  testArray<128>();
  testArray<255>();
  testArray<256>();
  testArray<999>();
}
