/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::ByteArray;
using scale::DecodeError;
using scale::EncodeError;
using scale::impl::memory::decode;
using scale::impl::memory::encode;

// Maximum available weight integer
using Compact = scale::Compact<boost::multiprecision::uint1024_t>;

/**
 * value parameterized tests
 */
class CompactTest
    : public testing::TestWithParam<std::pair<Compact, ByteArray>> {
 public:
  static std::pair<Compact, ByteArray> pair(Compact v, ByteArray m) {
    return std::make_pair(Compact(std::move(v)), std::move(m));
  }
};

/**
 * @given a value and corresponding buffer match of its encoding
 * @when given value being encoded
 * @then encoded value matches predefined buffer
 */
TEST_P(CompactTest, EncodeSuccess) {
  const auto &[value, expected] = GetParam();
  ASSERT_OUTCOME_SUCCESS(actual, encode(value));
  ASSERT_EQ(actual, expected) << "Encoding fail";
}

/**
 * @given a value and corresponding bytesof its encoding
 * @when value is decoded from given bytes
 * @then decoded value matches predefined value
 */
TEST_P(CompactTest, DecodeSuccess) {
  const auto &[expected, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(actual, decode<Compact>(bytes));
  ASSERT_EQ(actual, expected) << "Decoding fail";
}

#ifdef JAM_COMPATIBILITY_ENABLED

#define BIGGEST_INT_FOR_COMPACT_REPRESENTATION \
  ((Compact(1) << (8 * sizeof(size_t))) - 1)

INSTANTIATE_TEST_SUITE_P(
    CompactTestCases,
    CompactTest,
    ::testing::Values(
        // clang-format off

        // Lowest values for each number of bytes
  /*  0 */ CompactTest::pair(                                                               0b00000000, {0b00000000}),
  /*  1 */ CompactTest::pair(                                                               0b10000000, {0b10000000, 0b10000000}),
  /*  2 */ CompactTest::pair(                                                      0b01000000'00000000, {0b11000000, 0b00000000, 0b01000000}),
  /*  3 */ CompactTest::pair(                                             0b00100000'00000000'00000000, {0b11100000, 0b00000000, 0b00000000, 0b00100000}),
  /*  4 */ CompactTest::pair(                                    0b00010000'00000000'00000000'00000000, {0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00010000}),
  /*  5 */ CompactTest::pair(                           0b00001000'00000000'00000000'00000000'00000000, {0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001000}),
  /*  6 */ CompactTest::pair(                   0b0000100'00000000'00000000'00000000'00000000'00000000, {0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000100}),
  /*  7 */ CompactTest::pair(          0b0000010'00000000'00000000'00000000'00000000'00000000'00000000, {0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010}),
  /*  8 */ CompactTest::pair(0b00000001'00000000'00000000'00000000'00000000'00000000'00000000'00000000, {0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001}),

        // Biggest values for each number of bytes
  /*  9 */ CompactTest::pair(                                                               0b01111111, {0b01111111}),
  /* 10 */ CompactTest::pair(                                                      0b00111111'11111111, {0b10111111, 0b11111111}),
  /* 11 */ CompactTest::pair(                                             0b00011111'11111111'11111111, {0b11011111, 0b11111111, 0b11111111}),
  /* 12 */ CompactTest::pair(                                    0b00001111'11111111'11111111'11111111, {0b11101111, 0b11111111, 0b11111111, 0b11111111}),
  /* 13 */ CompactTest::pair(                           0b00000111'11111111'11111111'11111111'11111111, {0b11110111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 14 */ CompactTest::pair(                  0b00000011'11111111'11111111'11111111'11111111'11111111, {0b11111011, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 15 */ CompactTest::pair(         0b00000001'11111111'11111111'11111111'11111111'11111111'11111111, {0b11111101, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 16 */ CompactTest::pair(0b00000000'11111111'11111111'11111111'11111111'11111111'11111111'11111111, {0b11111110, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111}),
  /* 17 */ CompactTest::pair(0b11111111'11111111'11111111'11111111'11111111'11111111'11111111'11111111, {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111})
        // clang-format on
        ));

#else

#define BIGGEST_INT_FOR_COMPACT_REPRESENTATION ((Compact(1) << (8 * 67)) - 1)

INSTANTIATE_TEST_SUITE_P(
    CompactTestCases,
    CompactTest,
    ::testing::Values(
        // clang-format off
        // 0: 0 is min compact integer value, negative values are not allowed
        CompactTest::pair(0, {0}),
        // 1: 1 is encoded as 4
        CompactTest::pair(1, {4}),
        // 2: max 1 byte value
        CompactTest::pair(63, {252}),
        // 3: min 2 bytes value
        CompactTest::pair(64, {1, 1}),
        // 4: some 2 bytes value
        CompactTest::pair(255, {253, 3}),
        // 5: some 2 bytes value
        CompactTest::pair(511, {253, 7}),
        // 6: max 2 bytes value
        CompactTest::pair(16383, {253, 255}),
        // 7: min 4 bytes value
        CompactTest::pair(16384, {2, 0, 1, 0}),
        // 8: some 4 bytes value
        CompactTest::pair(65535, {254, 255, 3, 0}),
        // 9: max 4 bytes value
        CompactTest::pair(1073741823ul,   {0b1111'1110, 0xff, 0xff, 0xff}),
        // 10: min multibyte integer
        CompactTest::pair(1073741824,     {0b0000'0011, 0x00, 0x00, 0x00, 0b0100'0000}),
        // 11: some multibyte integer
        CompactTest::pair(1ull<<35,       {0b0000'0111, 0x00, 0x00, 0x00, 0x00, 0b0000'1000 }),
        // 12: some multibyte integer
        CompactTest::pair((1ull<<35)+1,   {0b0000'0111, 0x01, 0x00, 0x00, 0x00, 0b0000'1000 }),
        // 13: max multibyte integer:  2^536 - 1
        CompactTest::pair(BIGGEST_INT_FOR_COMPACT_REPRESENTATION,
                          std::vector<uint8_t>(68, 0xFF))));

#endif

/**
 * @given a CompactInteger value exceeding the range supported by scale
 * @when encode it directly as CompactInteger
 * @then obtain kValueIsTooBig error
 */
TEST(CompactTest, EncodeOutOfRangeBigIntegerFails) {
  // try to encode out of range big integer value MAX_BIGINT + 1
  // too big value, even for big integer case
  // we are going to have kValueIsTooBig error
  Compact v = BIGGEST_INT_FOR_COMPACT_REPRESENTATION + 1;
  ASSERT_OUTCOME_ERROR(
    encode(v),
    EncodeError::VALUE_TOO_BIG_FOR_COMPACT_REPRESENTATION);
}

/**
 * @given incorrect byte array, which assumes 4-th case of encoding
 * @when apply decodeInteger
 * @then get kNotEnoughData error
 */
TEST(CompactTest, compactDecodeBigIntegerError) {
  auto bytes = ByteArray{0xff, 0xff, 0xff, 0xff};
  ASSERT_OUTCOME_ERROR(decode<Compact>(bytes),
                       DecodeError::NOT_ENOUGH_DATA);
}

/**
 * @given redundant bytes in compact encoding
 * @when decode compact
 * @then error
 */
struct RedundantCompactTest : testing::TestWithParam<ByteArray> {};
TEST_P(RedundantCompactTest, DecodeError) {
  ASSERT_OUTCOME_ERROR(decode<Compact>(GetParam()),
                       DecodeError::REDUNDANT_COMPACT_ENCODING);
}

#ifdef JAM_COMPATIBILITY_ENABLED

INSTANTIATE_TEST_SUITE_P(
    RedundantCompactTestCases,
    RedundantCompactTest,
    ::testing::Values(
        // clang-format off
  /*  0 */ ByteArray{0b10000000, 0b00000000},
  /*  1 */ ByteArray{0b10000000, 0b00111111},
  /*  2 */ ByteArray{0b11000000, 0b00000000, 0b00000000},
  /*  3 */ ByteArray{0b11000000, 0b11111111, 0b00011111},
  /*  4 */ ByteArray{0b11100000, 0b00000000, 0b00000000, 0b00000000},
  /*  5 */ ByteArray{0b11100000, 0b11111111, 0b11111111, 0b00001111},
  /*  6 */ ByteArray{0b11110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /*  7 */ ByteArray{0b11110000, 0b11111111, 0b11111111, 0b11111111, 0b00000111},
  /*  8 */ ByteArray{0b11111000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /*  9 */ ByteArray{0b11111000, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000011},
  /* 10 */ ByteArray{0b11111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /* 11 */ ByteArray{0b11111100, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000001},
  /* 12 */ ByteArray{0b11111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /* 13 */ ByteArray{0b11111110, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000},
  /* 14 */ ByteArray{0b11111111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
  /* 15 */ ByteArray{0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000000}
     // // clang-format on
        ));

#else

INSTANTIATE_TEST_SUITE_P(
    RedundantCompactTestCases,
    RedundantCompactTest,
    ::testing::Values(ByteArray{0b100000'01, 0},
                      ByteArray{0b000000'10, 0b10000000, 0, 0},
                      ByteArray{0b000000'11, 0, 0, 0, 0b00'100000},
                      ByteArray{0b000001'11, 0, 0, 0, 0b01'000000, 0}));

#endif
