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
using Encoder = scale::backend::ToBytes<>;
using Decoder = scale::backend::FromBytes<>;

/**
 * @given byte array of 3 items: 0, 1, 2
 * @when create BasicStream wrapping this array and start to get bytes one by
 * one
 * @then bytes 0, 1, 2 are obtained sequentially @and take call returns error
 */
TEST(Decoder, TakeNextByte) {
  auto bytes = ByteArray{0, 1, 2};
  Decoder decoder{bytes};

  auto take_next = [&]() -> outcome::result<int> {
    try {
      return outcome::success(decoder.take());
    } catch (std::system_error &e) {
      return outcome::failure(e.code());
    }
  };

  for (size_t i = 0; i <= bytes.size(); ++i) {
    if (i < bytes.size()) {
      ASSERT_TRUE(decoder.has(1));
      ASSERT_OUTCOME_SUCCESS(byte, take_next());
      ASSERT_EQ(byte, bytes[i]);
    } else {
      ASSERT_FALSE(decoder.has(1));
      ASSERT_OUTCOME_ERROR(take_next(), DecodeError::NOT_ENOUGH_DATA);
    }
  }
}
