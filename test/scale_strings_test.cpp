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
 * @given string_view
 * @when specified string is encoded
 * @then encoded value meets expectations
 */
TEST(String, StringViewEncode) {
  constexpr std::string_view view = "abcdef";
  ASSERT_OUTCOME_SUCCESS(encoded, encode(view));

  auto encoded_without_size = std::span(encoded).last(encoded.size() - 1);
  auto expected = ByteArray{'a', 'b', 'c', 'd', 'e', 'f'};

  ASSERT_TRUE(std::ranges::equal(encoded_without_size, expected));
}

/**
 * @given raw string
 * @when specified string is encoded
 * @then encoded value meets expectations
 */
TEST(String, StringEncode) {
  std::string string = "abcdef";
  ASSERT_OUTCOME_SUCCESS(encoded, encode(string));

  auto actual = std::span(encoded).last(encoded.size() - 1);
  auto expected = ByteArray{'a', 'b', 'c', 'd', 'e', 'f'};

  ASSERT_TRUE(std::ranges::equal(actual, expected));
}

/**
 * @given byte array containing encoded string
 * @when string is decoded
 * @then decoded string matches expectations
 */
TEST(String, StringDecode) {
  std::string string = "abcdef";

  ASSERT_OUTCOME_SUCCESS(encoded, encode(string));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::string>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, string));
}
