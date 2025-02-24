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

template <typename T>
class IntegerTest : public testing::TestWithParam<std::pair<T, ByteArray>> {
 public:
  static std::pair<T, ByteArray> make_pair(const T &value,
                                           const ByteArray &match) {
    return std::make_pair(value, match);
  }

  using value_type = T;
};

/**
 * @brief class for testing int8_t encode and decode
 */
class Int8Test : public IntegerTest<int8_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Int8Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

TEST_P(Int8Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(Int8TestCases,
                         Int8Test,
                         ::testing::Values(Int8Test::make_pair(0, {0}),
                                           Int8Test::make_pair(-1, {255}),
                                           Int8Test::make_pair(-128, {128}),
                                           Int8Test::make_pair(-127, {129}),
                                           Int8Test::make_pair(123, {123}),
                                           Int8Test::make_pair(-15, {241})));

/**
 * @brief class for testing uint8_t encode and decode
 */
class Uint8Test : public IntegerTest<uint8_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Uint8Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Uint8Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(Uint8TestCases,
                         Uint8Test,
                         ::testing::Values(Uint8Test::make_pair(0, {0}),
                                           Uint8Test::make_pair(234, {234}),
                                           Uint8Test::make_pair(255, {255})));

/**
 * @brief class for testing int16_t encode and decode
 */
class Int16Test : public IntegerTest<int16_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Int16Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Int16Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(
    Int16TestCases,
    Int16Test,
    ::testing::Values(Int16Test::make_pair(-32767, {1, 128}),
                      Int16Test::make_pair(-32768, {0, 128}),
                      Int16Test::make_pair(-1, {255, 255}),
                      Int16Test::make_pair(32767, {255, 127}),
                      Int16Test::make_pair(12345, {57, 48}),
                      Int16Test::make_pair(-12345, {199, 207})));

/**
 * @brief class for testing uint16_t encode and decode
 */
class Uint16Test : public IntegerTest<uint16_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Uint16Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Uint16Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(
    Uint16TestCases,
    Uint16Test,
    ::testing::Values(Uint16Test::make_pair(32767, {255, 127}),
                      Uint16Test::make_pair(12345, {57, 48})));

/**
 * @brief class for testing int32_t encode and decode
 */
class Int32Test : public IntegerTest<int32_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Int32Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Int32Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(
    Int32TestCases,
    Int32Test,
    ::testing::Values(Int32Test::make_pair(2147483647l, {255, 255, 255, 127}),
                      Int32Test::make_pair(-1, {255, 255, 255, 255}),
                      Int32Test::make_pair(1, {1, 0, 0, 0})));

/**
 * @brief class for testing uint32_t encode and decode
 */
class Uint32Test : public IntegerTest<uint32_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Uint32Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Uint32Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(
    Uint32TestCases,
    Uint32Test,
    ::testing::Values(Uint32Test::make_pair(16909060ul, {4, 3, 2, 1}),
                      Uint32Test::make_pair(67305985, {1, 2, 3, 4})));

/**
 * @brief class for testing int64_t encode and decode
 */
class Int64Test : public IntegerTest<int64_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Int64Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Int64Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(
    Int64TestCases,
    Int64Test,
    ::testing::Values(
        Int64Test::make_pair(578437695752307201ll, {1, 2, 3, 4, 5, 6, 7, 8}),
        Int64Test::make_pair(-1, {255, 255, 255, 255, 255, 255, 255, 255})));

/**
 * @brief class for testing uint64_t encode and decode
 */
class Uint64Test : public IntegerTest<uint64_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Uint64Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Uint64Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(Uint64TestCases,
                         Uint64Test,
                         ::testing::Values(Uint64Test::make_pair(
                             578437695752307201ull, {1, 2, 3, 4, 5, 6, 7, 8})));

/**
 * @brief class for testing uint128_t encode and decode
 */
class Uint128Test : public IntegerTest<scale::uint128_t> {};

/**
 * @given a number and match buffer
 * @when given number being encoded
 * @then resulting buffer matches predefined one
 */
TEST_P(Uint128Test, EncodeSuccess) {
  auto [value, match] = GetParam();
  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, match);
}

/**
 * @given encoded sequence and match number
 * @when a number is decoded from given bytes
 * @then resulting number matches predefined one
 */
TEST_P(Uint128Test, DecodeSuccess) {
  auto [match, bytes] = GetParam();
  ASSERT_OUTCOME_SUCCESS(decoded, decode<value_type>(bytes));
  ASSERT_EQ(decoded, match);
}

INSTANTIATE_TEST_SUITE_P(
    Uint128TestCases,
    Uint128Test,
    ::testing::Values(
        // clang-format off
        Uint128Test::make_pair(
            scale::uint128_t("1"), // 1
            {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}),
        Uint128Test::make_pair(
            scale::uint128_t("18446744073709551616"), // 2^64
            {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}),
        Uint128Test::make_pair( // 2^128-1 (uint128 max)
            scale::uint128_t("340282366920938463463374607431768211455"),
            {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff})
        // clang-format on
        ));
