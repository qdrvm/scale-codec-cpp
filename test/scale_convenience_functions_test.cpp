/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <qtils/test/outcome.hpp>

#include <scale/scale.hpp>

using scale::impl::with_buffer::decode;
using scale::impl::with_buffer::encode;

struct TestStruct {
  std::string a;
  int b;
  bool operator==(const TestStruct &other) const = default;
};

/**
 * @given encoded TestStruct
 * @when it is decoded back
 * @then we get original TestStruct
 */
TEST(ScaleConvenienceFuncsTest, EncodeSingleValidArgTest) {
  TestStruct s1{"some_string", 42};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(s1));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<TestStruct>(encoded));

  ASSERT_EQ(decoded, s1);
}

/**
 * @given encoded string and integer using scale::encode with variadic template
 * @when we decode encoded vector back and put those fields to TestStruct
 * @then we get original string and int
 */
TEST(ScaleConvenienceFuncsTest, EncodeSeveralValidArgTest) {
  std::string expected_string = "some_string";
  int expected_int = 42;

  ASSERT_OUTCOME_SUCCESS(encoded,
                         encode(std::tie(expected_string, expected_int)));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<TestStruct>(encoded));

  ASSERT_EQ(decoded.a, expected_string);
  ASSERT_EQ(decoded.b, expected_int);
}
