/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <qtils/test/outcome.hpp>

#include <scale/scale.hpp>

using scale::ByteArray;
using scale::EncodeError;
using scale::impl::with_buffer::decode;
using scale::impl::with_buffer::encode;

struct X {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  bool operator==(const X &other) const = default;
};

TEST(SmartPointersAndRef, SharedPointer_SuccessEncode) {
  std::shared_ptr<X> value = std::make_shared<X>(0xde, 0xad, 0xbe, 0xef);
  ByteArray expected = {0xde, 0xad, 0xbe, 0xef};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, expected);
}

TEST(SmartPointersAndRef, SharedPointer_FailEncode_Nullptr) {
  std::shared_ptr<X> value;

  ASSERT_OUTCOME_ERROR(encode(value), EncodeError::DEREF_NULLPOINTER);
}

TEST(SmartPointersAndRef, SharedPointer_SuccessDecode) {
  ByteArray bytes = {0xde, 0xad, 0xbe, 0xef};
  X expected{0xde, 0xad, 0xbe, 0xef};

  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::shared_ptr<X>>(bytes));
  ASSERT_TRUE(decoded != nullptr);
  ASSERT_EQ(*decoded, expected);
}

TEST(SmartPointersAndRef, UniquePointer_SuccessEncode) {
  std::unique_ptr<X> value = std::make_unique<X>(0xde, 0xad, 0xbe, 0xef);
  ByteArray expected = {0xde, 0xad, 0xbe, 0xef};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(value));
  ASSERT_EQ(encoded, expected);
}

TEST(SmartPointersAndRef, UniquePointer_FailEncode_Nullptr) {
  std::unique_ptr<X> value;

  ASSERT_OUTCOME_ERROR(encode(value), EncodeError::DEREF_NULLPOINTER);
}

TEST(SmartPointersAndRef, UniquePointer_SuccessDecode) {
  ByteArray bytes = {0xde, 0xad, 0xbe, 0xef};
  X expected{0xde, 0xad, 0xbe, 0xef};

  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::unique_ptr<X>>(bytes));
  ASSERT_TRUE(decoded != nullptr);
  ASSERT_EQ(*decoded, expected);
}

TEST(SmartPointersAndRef, RefWrapper_SuccessEncode) {
  X value = {0xde, 0xad, 0xbe, 0xef};
  ByteArray expected = {0xde, 0xad, 0xbe, 0xef};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(std::cref(value)));
  ASSERT_EQ(encoded, expected);
}
