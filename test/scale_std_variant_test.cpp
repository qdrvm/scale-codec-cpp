/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <qtils/test/outcome.hpp>

#include <scale/scale.hpp>

using scale::ByteArray;
using scale::get_variant;
using scale::impl::memory::decode;
using scale::impl::memory::encode;
using Encoder = scale::Encoder<scale::backend::ToBytes>;
using Decoder = scale::Decoder<scale::backend::FromBytes>;

using Testee = std::variant<uint8_t, uint32_t>;

class StdVariantFixture
    : public testing::TestWithParam<std::pair<Testee, ByteArray>> {};

namespace {
  std::pair<Testee, ByteArray> make_pair(Testee variant, ByteArray bytes) {
    return {variant, std::move(bytes)};
  }
}  // namespace

/**
 * @given variant value and byte array
 * @when value is scale-encoded
 * @then encoded bytes match predefined byte array
 */
TEST_P(StdVariantFixture, EncodeSuccess) {
  const auto &[value, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, bytes);
}

/**
 * @given variant value and byte array
 * @when value is scale-encoded
 * @then encoded bytes match predefined byte array
 */
TEST_P(StdVariantFixture, DecodeSuccess) {
  const auto &[value, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(bytes));
  ASSERT_EQ(decoded, value);
}

INSTANTIATE_TEST_SUITE_P(CompactTestCases,
                         StdVariantFixture,
                         ::testing::Values(make_pair(uint8_t(1), {0, 1}),
                                           make_pair(uint32_t(2),
                                                     {1, 2, 0, 0, 0})));

/**
 * @given byte array of encoded variant of types uint8_t and uint32_t
 * containing uint8_t value
 * @when variant decoded from scale decoder stream
 * @then obtained varian has alternative type uint8_t and is equal to encoded
 * uint8_t value
 */
TEST(ScaleStdVariant, DecodeU8Success) {
  ByteArray bytes = {0, 1};  // uint8_t{1}
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(bytes));
  ASSERT_EQ(scale::get_variant<uint8_t>(decoded), 1);
}

/**
 * @given byte array of encoded variant of types uint8_t and uint32_t
 * containing uint32_t value
 * @when variant decoded from scale decoder stream
 * @then obtained varian has alternative type uint32_t and is equal to encoded
 * uint32_t value
 */
TEST(ScaleStdVariant, DecodeU32Success) {
  ByteArray bytes = {1, 1, 0, 0, 0};  // uint32_t{1}
  ASSERT_OUTCOME_SUCCESS(decoded, decode<Testee>(bytes));
  ASSERT_EQ(scale::get_variant<uint32_t>(decoded), 1);
}

/**
 * @given byte array of encoded variant of types uint8_t and uint32_t
 * containing uint32_t value
 * @when variant decoded from scale decoder stream
 * @then obtained varian has alternative type uint32_t and is equal to encoded
 * uint32_t value
 */
TEST(ScaleStdVariant, DecodeWrongVariantIndex) {
  ByteArray bytes = {2, 0, 0, 0, 0};  // index out of bound
  ASSERT_OUTCOME_ERROR(decode<Testee>(bytes),
                       scale::DecodeError::WRONG_TYPE_INDEX);
}
