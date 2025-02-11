/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::ByteArray;
using scale::impl::memory::decode;
using scale::impl::memory::encode;

/**
 * @given pair of values of different types: uint8_t and uint32_t
 * @when encode is applied
 * @then obtained serialized value meets predefined one
 */
TEST(Pair, Encode) {
  uint8_t v1 = 1;
  uint32_t v2 = 2;

  ASSERT_OUTCOME_SUCCESS(encoded, encode(std::make_pair(v1, v2)));
  ASSERT_EQ(encoded, (ByteArray{1, 2, 0, 0, 0}));
}

/**
 * @given byte sequence containign 2 encoded values of
 * different types: uint8_t and uint32_t
 * @when decode is applied
 * @then obtained pair mathces predefined one
 */
TEST(Pair, Decode) {
  ByteArray bytes = {1, 2, 0, 0, 0};

  using Pair = std::pair<uint8_t, uint32_t>;
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Pair>(bytes));
  ASSERT_EQ(decoded.first, 1);
  ASSERT_EQ(decoded.second, 2);
}
