
/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Unit tests for encoding and decoding tagged types using SCALE.
 *
 * This file contains tests for encoding and decoding tagged types,
 * ensuring that serialization and deserialization work as expected.
 */

#include <gtest/gtest.h>

#include <qtils/tagged.hpp>
#include <qtils/test/outcome.hpp>

#include <scale/scale.hpp>

using scale::ByteArray;
using scale::impl::memory::decode;
using scale::impl::memory::encode;

using TaggedString = qtils::Tagged<std::string, class String>;
using TaggedInteger = qtils::Tagged<uint32_t, class Integer>;

/**
 * @brief Tests encoding of a tagged string.
 *
 * @given A string and its tagged equivalent.
 * @when Encoding is applied.
 * @then The serialized value of the tagged string matches the original string.
 */
TEST(Tagged, StringEncode) {
  std::string string = "hello world";
  TaggedString tagged = "hello world";

  ASSERT_OUTCOME_SUCCESS(encoded_original, encode(string));
  ASSERT_OUTCOME_SUCCESS(encoded_tagged, encode(tagged));

  ASSERT_EQ(encoded_tagged, encoded_original);
}

/**
 * @brief Tests decoding of a tagged string.
 *
 * @given A byte sequence containing an encoded string.
 * @when Decoding is applied.
 * @then The decoded value matches the original string.
 */
TEST(Tagged, StringDecode) {
  std::string original = "hello world";
  ASSERT_OUTCOME_SUCCESS(encoded, encode(original));

  ASSERT_OUTCOME_SUCCESS(decoded, decode<TaggedString>(encoded));

  ASSERT_EQ(untagged(decoded), original);
}

/**
 * @brief Tests encoding and decoding of a tagged integer.
 *
 * @given A tagged integer.
 * @when The integer is encoded and then decoded.
 * @then The decoded value matches the original integer.
 */
TEST(Tagged, StringEncodeAndDecode) {
  TaggedString original = "hello world";

  ASSERT_OUTCOME_SUCCESS(encoded, encode(original));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<TaggedString>(encoded));
  ASSERT_EQ(decoded, original);
}

TEST(Tagged, IntegerEncode) {
  uint32_t integer = 123456789;
  TaggedInteger tagged(123456789);

  ASSERT_OUTCOME_SUCCESS(encoded_original, encode(integer));
  ASSERT_OUTCOME_SUCCESS(encoded_tagged, encode(tagged));

  ASSERT_EQ(encoded_tagged, encoded_original);
}

/**
 * @brief Tests decoding of a tagged integer.
 *
 * @given A byte sequence containing an encoded integer.
 * @when Decoding is applied.
 * @then The decoded value matches the original integer.
 */
TEST(Tagged, IntegerDecode) {
  uint32_t original = 123456789;
  ASSERT_OUTCOME_SUCCESS(encoded, encode(original));

  ASSERT_OUTCOME_SUCCESS(decoded, decode<TaggedInteger>(encoded));

  ASSERT_EQ(untagged(decoded), original);
}