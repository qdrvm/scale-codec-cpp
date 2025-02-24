/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::backend::ForCount;

template <typename T>
outcome::result<size_t> encode(T &&value) {
  ForCount encoder;
  try {
    encode(std::forward<T>(value), encoder);
  } catch (std::system_error &e) {
    return outcome::failure(e.code());
  }
  return std::move(encoder).size();
}

class ScaleCounter : public testing::Test {
 protected:
};

struct TestStruct {
  uint8_t x;
  std::string y;
};

/**
 * @given a bool
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, Bool) {
  ASSERT_OUTCOME_SUCCESS(count, encode(true));
  ASSERT_EQ(count, 1);
}

/**
 * @given a string
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, String) {
  std::string value = "test string";
  ASSERT_OUTCOME_SUCCESS(count, encode(value));
  ASSERT_EQ(count, value.size() + 1);
}

/**
 * @given an empty optional
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, EmptyOptional) {
  std::optional<uint32_t> var = std::nullopt;
  ASSERT_OUTCOME_SUCCESS(count, encode(var));
  ASSERT_EQ(count, 1);
}

/**
 * @given an optional with an uint32 value inside
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, NonEmptyOptional) {
  std::optional<uint32_t> var = 10;
  ASSERT_OUTCOME_SUCCESS(count, encode(var));
  ASSERT_EQ(count, 5);
}

/**
 * @given a custom defined struct
 * @when it gets scale encoded
 * @then the resulting stream size equals to expected
 */
TEST_F(ScaleCounter, CustomStruct) {
  TestStruct st{.x = 10, .y = "test string"};
  ASSERT_OUTCOME_SUCCESS(count, encode(st));
  ASSERT_EQ(count, 1 + st.y.size() + 1);
}
