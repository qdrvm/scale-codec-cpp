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
using Decoder = scale::Decoder<scale::backend::FromBytes>;

/**
 * @given variety of optional values
 * @when encodeOptional function is applied
 * @then expected result obtained
 */
TEST(Optional, Encode) {
  // most simple case
  {
    ASSERT_OUTCOME_SUCCESS(encoded,
                           encode(std::optional<uint8_t>{std::nullopt}));
    ASSERT_EQ(encoded, (ByteArray{0}));
  }

  // encode existing uint8_t
  {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(std::optional<uint8_t>{1}));
    ASSERT_EQ(encoded, (ByteArray{1, 1}));
  }

  // encode negative int8_t
  {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(std::optional<uint8_t>{-1}));
    ASSERT_EQ(encoded, (ByteArray{1, 255}));
  }

  // encode non-existing uint16_t
  {
    ASSERT_OUTCOME_SUCCESS(encoded,
                           encode(std::optional<uint16_t>{std::nullopt}));
    ASSERT_EQ(encoded, (ByteArray{0}));
  }

  // encode existing uint16_t
  {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(std::optional<uint16_t>{511}));
    ASSERT_EQ(encoded, (ByteArray{1, 255, 1}));
  }

  // encode existing uint32_t
  {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(std::optional<uint32_t>{67305985}));
    ASSERT_EQ(encoded, (ByteArray{1, 1, 2, 3, 4}));
  }
}

/**
 * @given byte stream containing series of encoded optional values
 * @when decodeOptional function sequencially applied
 * @then expected values obtained
 */
TEST(Optional, DecodeSuccess) {
  // clang-format off
    auto bytes = ByteArray{
            0,              // first value
            1, 1,           // second value
            1, 255,         // third value
            0,              // fourth value
            1, 255, 1,      // fifth value
            1, 1, 2, 3, 4}; // sixth value
  // clang-format on

  auto decoder = Decoder{bytes};

  // decode nullopt uint8_t
  {
    std::optional<uint8_t> opt;
    ASSERT_NO_THROW((decoder >> opt));
    ASSERT_FALSE(opt.has_value());
  }

  // decode optional uint8_t
  {
    std::optional<uint8_t> opt;
    ASSERT_NO_THROW((decoder >> opt));
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(*opt, 1);
  }

  // decode optional negative int8_t
  {
    std::optional<int8_t> opt;
    ASSERT_NO_THROW((decoder >> opt));
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(*opt, -1);
  }

  // decode nullopt uint16_t
  // it requires 1 zero byte just like any other nullopt
  {
    std::optional<uint16_t> opt;
    ASSERT_NO_THROW((decoder >> opt));
    ASSERT_FALSE(opt.has_value());
  }

  // decode optional uint16_t
  {
    std::optional<uint16_t> opt;
    ASSERT_NO_THROW((decoder >> opt));
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(*opt, 511);
  }

  // decode optional uint32_t
  {
    std::optional<uint32_t> opt;
    ASSERT_NO_THROW((decoder >> opt));
    ASSERT_TRUE(opt.has_value());
    ASSERT_EQ(*opt, 67305985);
  }
}

TEST(Optional, DecodeFail) {
  {
    auto bytes = ByteArray{2, 0, 0};
    ASSERT_OUTCOME_ERROR(decode<std::optional<uint8_t>>(bytes),
                         DecodeError::UNEXPECTED_VALUE);
  }
  {
    auto bytes = ByteArray{1, 0, 0};
    ASSERT_OUTCOME_ERROR(decode<std::optional<uint32_t>>(bytes),
                         DecodeError::NOT_ENOUGH_DATA);
  }
}

/**
 * optional bool tests
 */

/**
 * @given optional bool values: true, false, nullopt
 * @when encode optional is applied
 * @then expected result obtained
 */
TEST(OptionalBool, Encode) {
  std::array<std::optional<bool>, 3> values = {std::nullopt, true, false};
  ASSERT_OUTCOME_SUCCESS(encoded, encode(values));
  ASSERT_EQ(encoded, (ByteArray{0, 1, 2}));
}

/**
 * @brief helper struct for testing decode optional bool
 */
struct FourOptBools {
  std::optional<bool> b1;
  std::optional<bool> b2;
  std::optional<bool> b3;
  std::optional<bool> b4;
};

/**
 * @given byte array containing series of encoded optional bool values
 * where last byte is incorrect for optional bool type
 * @when scale::decode function is applied
 * @then DecoderError::UNEXPECTED_VALUE error is obtained
 */
TEST(OptionalBool, DecodeFail) {
  auto bytes = ByteArray{0, 1, 2, 3};

  ASSERT_OUTCOME_ERROR(decode<FourOptBools>(bytes),
                       DecodeError::UNEXPECTED_VALUE);
}

/**
 * @given byte array containing series of encoded optional bool values
 * @when scale::decode function is applied
 * @then obtained values meet expectations
 */
TEST(OptionalBool, DecodeSuccess) {
  auto bytes = ByteArray{0, 1, 2, 1};

  ASSERT_OUTCOME_SUCCESS(res, decode<FourOptBools>(bytes));
  ASSERT_TRUE(res.b1 == std::nullopt);
  ASSERT_TRUE(res.b2 == true);
  ASSERT_TRUE(res.b3 == false);
  ASSERT_TRUE(res.b4 == true);
}

/**
 * @given encode stream
 * @when encode nullopt by push it into encode stream
 * @then obtained byte array with added only one zero-byte
 */
TEST(Nullopt, Encode) {
  ASSERT_OUTCOME_SUCCESS(encoded, encode(std::nullopt));
  ASSERT_EQ(encoded, (ByteArray{0}));
}

/**
 * @given byte array with only one zero-byte
 * @when decode it to any optional type
 * @then obtained optional with nullopt value
 */
TEST(Nullopt, Decode) {
  ByteArray encoded_nullopt{0};

  using OptionalInt = std::optional<int>;
  ASSERT_OUTCOME_SUCCESS(int_opt, decode<OptionalInt>(encoded_nullopt));
  EXPECT_EQ(int_opt, std::nullopt);

  using OptionalTuple = std::optional<std::tuple<int, int>>;
  ASSERT_OUTCOME_SUCCESS(tuple_opt, decode<OptionalTuple>(encoded_nullopt));
  EXPECT_EQ(tuple_opt, std::nullopt);
}
