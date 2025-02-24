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

/**
 * @given bool values: true and false
 * @when encode them by fixedwidth::encodeBool function
 * @then obtain expected result each time
 */
TEST(ScaleBoolTest, EncodeBoolSuccess) {
  {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(true));
    ASSERT_EQ(encoded, ByteArray{0x1});
  }
  {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(false));
    ASSERT_EQ(encoded, ByteArray{0x0});
  }
}

/**
 * @brief helper structure for testing scale::decode
 */
struct ThreeBooleans {
  bool b1 = false;
  bool b2 = false;
  bool b3 = false;
};

/**
 * @given byte array containing values {0, 1, 2}
 * @when scale::decode function is applied sequentially
 * @then it returns false, true and kUnexpectedValue error correspondingly,
 * and in the end no more bytes left in stream
 */
TEST(ScaleBoolTest, fixedwidthDecodeBoolFail) {
  auto bytes = ByteArray{0, 1, 2};
  ASSERT_OUTCOME_ERROR(decode<ThreeBooleans>(bytes),
                       DecodeError::UNEXPECTED_VALUE);
}

/**
 * @given byte array containing values {0, 1, 2}
 * @when scale::decode function is applied sequentially
 * @then it returns false, true and kUnexpectedValue error correspondingly,
 * and in the end no more bytes left in stream
 */
TEST(ScaleBoolTest, fixedwidthDecodeBoolSuccess) {
  auto bytes = ByteArray{0, 1, 0};
  ASSERT_OUTCOME_SUCCESS(decoded, decode<ThreeBooleans>(bytes));
  ASSERT_EQ(decoded.b1, false);
  ASSERT_EQ(decoded.b2, true);
  ASSERT_EQ(decoded.b3, false);
}
