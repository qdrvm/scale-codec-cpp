/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Unit tests for verifying the correctness of SCALE encoding and decoding
 *        for various decomposable data structures.
 *
 * This file contains test cases that check the encoding and decoding of
 * fundamental types, arrays, tuples, and user-defined structures using
 * the SCALE codec implementation.
 */

#include <gtest/gtest.h>

#include <set>

#include <qtils/test/outcome.hpp>
#include <scale/bitvec.hpp>
#include <scale/scale.hpp>

using scale::as_compact;
using scale::BitVec;
using scale::ByteArray;
using scale::CompactInteger;
using scale::DecodeError;
using scale::impl::memory::decode;
using scale::impl::memory::encode;
using Encoder = scale::backend::ToBytes;
using Decoder = scale::backend::FromBytes;

TEST(Decomposable, C_Style_array) {
  using Testee = const uint16_t[3];

  Testee value = {1, 2, 3};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  Decoder decoder(encoded);
  Testee decoded{};
  ASSERT_NO_THROW(decode(decoded, decoder));
  ASSERT_TRUE(std::ranges::equal(decoded, value));
}

TEST(Decomposable, Array) {
  using Testee = std::array<const uint16_t, 3>;

  Testee value{1, 2, 3};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(encoded));
  ASSERT_EQ(decoded, value);
}

TEST(Decomposable, Pair) {
  using Testee = std::pair<uint8_t, uint32_t>;

  Testee value = {13, 777};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(encoded));
  ASSERT_EQ(decoded, value);
}

/**
 * @given a tuple composed of 4 different values and corresponding byte array
 * @when tuple is encoded, @and then decoded
 * @then decoded value matches the original tuple
 */
TEST(Decomposable, Tuple) {
  using Testee = std::tuple<uint8_t, uint16_t, const uint32_t, const uint64_t>;

  Testee value = {1, 3, 2, 4};

  ASSERT_OUTCOME_SUCCESS(actual_bytes, encode(value));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(actual_bytes));
  ASSERT_EQ(decoded, value);
}

TEST(Decomposable, Tie) {
  uint8_t src1 = 13;
  const uint16_t src2 = 777;

  uint8_t dst1 = 0;
  const uint16_t dst2 = 0;

  Encoder encoder;
  ASSERT_NO_THROW(encode(std::tie(src1, src2), encoder));
  ASSERT_OUTCOME_SUCCESS(encoded, encode(std::tie(src1, src2)));

  Decoder decoder{encoded};
  ASSERT_NO_THROW(decode(std::tie(dst1, dst2), decoder));

  ASSERT_EQ(src1, dst1);
  ASSERT_EQ(src2, dst2);
}

TEST(Decomposable, Aggregate) {
  struct Testee {
    uint8_t m1 = 0;
    uint8_t m2 = 0;
    uint8_t m3 = 0;
    bool operator==(const Testee &) const = default;
  };

  Testee value{1, 2, 3};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(encoded));
  ASSERT_EQ(decoded, value);
}

struct CustomDecomposable {
  uint16_t m1 = 0;
  const uint16_t m2 = 0;
  uint16_t m3 = 0;
  const uint16_t m4 = 0;
  bool operator==(const CustomDecomposable &) const = default;
  SCALE_CUSTOM_DECOMPOSITION(CustomDecomposable,
                             m1,
                             /* m2 skipped */
                             as_compact(m3),
                             m4);
};

TEST(Decomposable, CustomDecomposable) {
  using Testee = CustomDecomposable;

  Testee value{1, 2, 3, 4};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(encoded));
  ASSERT_EQ(decoded.m1, value.m1);
  ASSERT_EQ(decoded.m2, CustomDecomposable{}.m2);  // by default, not affected
  ASSERT_EQ(decoded.m3, value.m3);
  ASSERT_EQ(decoded.m4, value.m4);

  std::vector<int> vec{};
  auto x = encode(vec.size());
}
