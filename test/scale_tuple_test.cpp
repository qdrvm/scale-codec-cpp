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
 * @given 3 values of different types: uint8_t, uint32_t and uint8_t
 * @when encode is applied
 * @then obtained serialized value meets predefined one
 */
TEST(Tuple, Encode) {
  uint8_t v1 = 1;
  uint16_t v2 = 2;
  uint32_t v3 = 3;

  ASSERT_OUTCOME_SUCCESS(encoded, encode(std::tie(v1, v2, v3)));

  ByteArray expected = {1, 2, 0, 3, 0, 0, 0};
  ASSERT_EQ(encoded, expected);
}

/**
 * @given byte sequence containign 3 encoded values of
 * different types: uint8_t, uint32_t and uint8_t
 * @when decode is applied
 * @then obtained pair matches predefined one
 */
TEST(Tuple, Decode) {
  ByteArray bytes = {1, 2, 0, 0, 0, 3};
  using Tuple = std::tuple<uint8_t, uint32_t, uint8_t>;

  ASSERT_OUTCOME_SUCCESS(decoded, decode<Tuple>(bytes));

  Tuple expected = {1, 2, 3};
  ASSERT_EQ(decoded, expected);
}

/**
 * @given a tuple composed of 4 different values and correspondent byte array
 * @when tuple is encoded, @and then decoded
 * @then decoded value come up with original tuple
 */
TEST(Tuple, EncodeAndDecode) {
  using Tuple = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t>;
  Tuple tuple = {1, 3, 2, 4};

  ASSERT_OUTCOME_SUCCESS(actual_bytes, encode(tuple));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Tuple>(actual_bytes));
  ASSERT_EQ(decoded, tuple);
}
