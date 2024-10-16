/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::ByteArray;
using scale::CompactInteger;
using scale::decode;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

/**
 * value parameterized tests
 */
class CompactTest
    : public ::testing::TestWithParam<std::pair<CompactInteger, ByteArray>> {
 public:
  static std::pair<CompactInteger, ByteArray> pair(CompactInteger v,
                                                   ByteArray m) {
    return std::make_pair(CompactInteger(std::move(v)), std::move(m));
  }

 protected:
  ScaleEncoderStream s;
};

/**
 * @given a value and corresponding buffer match of its encoding
 * @when value is encoded by means of ScaleEncoderStream
 * @then encoded value matches predefined buffer
 */
TEST_P(CompactTest, EncodeSuccess) {
  const auto &[value, match] = GetParam();
  ASSERT_NO_THROW(s << value);
  ASSERT_EQ(s.to_vector(), match);
}

/**
 * @given a value and corresponding bytesof its encoding
 * @when value is decoded by means of ScaleDecoderStream from given bytes
 * @then decoded value matches predefined value
 */
TEST_P(CompactTest, DecodeSuccess) {
  const auto &[value_match, bytes] = GetParam();
  ScaleDecoderStream s(bytes);
  CompactInteger v{};
  ASSERT_NO_THROW(s >> v);
  ASSERT_EQ(v, value_match);
}

INSTANTIATE_TEST_SUITE_P(
    CompactTestCases,
    CompactTest,
    ::testing::Values(
        // 0 is min compact integer value, negative values are not allowed
        CompactTest::pair(0, {0}),
        // 1 is encoded as 4
        CompactTest::pair(1, {4}),
        // max 1 byte value
        CompactTest::pair(63, {252}),
        // min 2 bytes value
        CompactTest::pair(64, {1, 1}),
        // some 2 bytes value
        CompactTest::pair(255, {253, 3}),
        // some 2 bytes value
        CompactTest::pair(511, {253, 7}),
        // max 2 bytes value
        CompactTest::pair(16383, {253, 255}),
        // min 4 bytes value
        CompactTest::pair(16384, {2, 0, 1, 0}),
        // some 4 bytes value
        CompactTest::pair(65535, {254, 255, 3, 0}),
        // max 4 bytes value
        CompactTest::pair(1073741823ul, {254, 255, 255, 255}),
        // some multibyte integer
        CompactTest::pair(
            CompactInteger("1234567890123456789012345678901234567890"),
            {0b110111,
             210,
             10,
             63,
             206,
             150,
             95,
             188,
             172,
             184,
             243,
             219,
             192,
             117,
             32,
             201,
             160,
             3}),
        // min multibyte integer
        CompactTest::pair(1073741824, {3, 0, 0, 0, 64}),
        // max multibyte integer
        CompactTest::pair(
            CompactInteger(
                "224945689727159819140526925384299092943484855915095831"
                "655037778630591879033574393515952034305194542857496045"
                "531676044756160413302774714984450425759043258192756735"),
            std::vector<uint8_t>(68, 0xFF))));

/**
 * Negative tests
 */

/**
 * @given a negative value -1
 * (negative values are not supported by compact encoding)
 * @when trying to encode this value
 * @then obtain error
 */
TEST(ScaleCompactTest, EncodeNegativeIntegerFails) {
  CompactInteger v(-1);
  ScaleEncoderStream out{};
  ASSERT_ANY_THROW((out << v));
  ASSERT_EQ(out.to_vector().size(), 0);  // nothing was written to buffer
}

/**
 * @given a CompactInteger value exceeding the range supported by scale
 * @when encode it a directly as CompactInteger
 * @then obtain kValueIsTooBig error
 */
TEST(ScaleCompactTest, EncodeOutOfRangeBigIntegerFails) {
  // try to encode out of range big integer value MAX_BIGINT + 1 == 2^536
  // too big value, even for big integer case
  // we are going to have kValueIsTooBig error
  CompactInteger v(
      "224945689727159819140526925384299092943484855915095831"
      "655037778630591879033574393515952034305194542857496045"
      "531676044756160413302774714984450425759043258192756736");  // 2^536

  ScaleEncoderStream out;
  ASSERT_ANY_THROW((out << v));          // value is too big, it is not encoded
  ASSERT_EQ(out.to_vector().size(), 0);  // nothing was written to buffer
}

/**
 * @given incorrect byte array, which assumes 4-th case of encoding
 * @when apply decodeInteger
 * @then get kNotEnoughData error
 */
TEST(Scale, compactDecodeBigIntegerError) {
  auto bytes = ByteArray{255, 255, 255, 255};
  EXPECT_EC(decode<CompactInteger>(bytes), scale::DecodeError::NOT_ENOUGH_DATA);
}

/**
 * @given redundant bytes in compact encoding
 * @when decode compact
 * @then error
 */
struct RedundantCompactTest : ::testing::TestWithParam<ByteArray> {};
TEST_P(RedundantCompactTest, DecodeError) {
  EXPECT_EC(scale::decode<CompactInteger>(GetParam()),
            scale::DecodeError::REDUNDANT_COMPACT_ENCODING);
}
INSTANTIATE_TEST_SUITE_P(
    RedundantCompactTestCases,
    RedundantCompactTest,
    ::testing::Values(ByteArray{0b100000'01, 0},
                      ByteArray{0b000000'10, 0b10000000, 0, 0},
                      ByteArray{0b000000'11, 0, 0, 0, 0b00'100000},
                      ByteArray{0b000001'11, 0, 0, 0, 0b01'000000, 0}));
