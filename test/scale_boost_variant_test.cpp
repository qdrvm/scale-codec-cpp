/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <scale/scale.hpp>

#ifdef USE_BOOST_VARIANT

using scale::ByteArray;
using scale::decode;
using scale::encode;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

class BoostVariantFixture
    : public testing::TestWithParam<
          std::pair<boost::variant<uint8_t, uint32_t>, ByteArray>> {
 protected:
  ScaleEncoderStream s;
};
namespace {
  std::pair<boost::variant<uint8_t, uint32_t>, ByteArray> make_pair(
      boost::variant<uint8_t, uint32_t> v, ByteArray m) {
    return std::pair<boost::variant<uint8_t, uint32_t>, ByteArray>(
        std::move(v), std::move(m));
  }
}  // namespace

/**
 * @given variant value and byte array
 * @when value is scale-encoded
 * @then encoded bytes match predefined byte array
 */
TEST_P(BoostVariantFixture, EncodeSuccessTest) {
  const auto &[value, match] = GetParam();
  ASSERT_NO_THROW(s << value);
  ASSERT_EQ(s.to_vector(), match);
}

INSTANTIATE_TEST_SUITE_P(CompactTestCases,
                         BoostVariantFixture,
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
TEST(ScaleBoostVariant, DecodeU8Success) {
  ByteArray match = {0, 1};  // uint8_t{1}
  ScaleDecoderStream s(match);
  boost::variant<uint8_t, uint32_t> val{};
  ASSERT_NO_THROW(s >> val);
  ASSERT_EQ(boost::get<uint8_t>(val), 1);
}

/**
 * @given byte array of encoded variant of types uint8_t and uint32_t
 * containing uint32_t value
 * @when variant decoded from scale decoder stream
 * @then obtained varian has alternative type uint32_t and is equal to encoded
 * uint32_t value
 */
TEST(ScaleBoostVariant, DecodeU32Success) {
  ByteArray match = {1, 1, 0, 0, 0};  // uint32_t{1}
  ScaleDecoderStream s(match);
  boost::variant<uint8_t, uint32_t> val{};
  ASSERT_NO_THROW(s >> val);
  ASSERT_EQ(boost::get<uint32_t>(val), 1);
}

#endif
