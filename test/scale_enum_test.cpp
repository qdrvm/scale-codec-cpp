/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::DecodeError;
using scale::impl::with_buffer::decode;
using scale::impl::with_buffer::encode;

template <typename T>
class ValidEnum : public testing::Test {
 protected:
  const static std::string enum_name;
  const static std::vector<T> values;
};

enum class Foo : uint16_t { A = 0, B = 1, C = 2 };

enum class Bar : int64_t { A = -32, B = 42, C = 0 };

using MyTypes = testing::Types<Foo, Bar>;
TYPED_TEST_SUITE(ValidEnum, MyTypes);

template <>
const std::string ValidEnum<Foo>::enum_name{"Foo"};

SCALE_DEFINE_ENUM_VALUE_RANGE(, Foo, Foo::A, Foo::C);

template <>
const std::vector<Foo> ValidEnum<Foo>::values{Foo::A, Foo::B, Foo::C};

template <>
const std::string ValidEnum<Bar>::enum_name{"Bar"};
template <>
const std::vector<Bar> ValidEnum<Bar>::values{Bar::A, Bar::B, Bar::C};

SCALE_DEFINE_ENUM_VALUE_LIST(, Bar, Bar::A, Bar::B, Bar::C);

TYPED_TEST(ValidEnum, ConsistentEncodingDecoding) {
  SCOPED_TRACE(TestFixture::enum_name);
  for (auto const &param : TestFixture::values) {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(param));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TypeParam>(encoded));
    EXPECT_EQ(decoded, param);
  }
}

TYPED_TEST(ValidEnum, CorrectEncoding) {
  using Type = std::underlying_type_t<TypeParam>;
  for (auto const &param : TestFixture::values) {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(param));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<Type>(encoded));
    EXPECT_EQ(decoded, static_cast<std::underlying_type_t<TypeParam>>(param));
  }
}

template <typename T>
class InvalidEnum : public testing::Test {
 protected:
  const static std::string enum_name;
  const static std::vector<std::underlying_type_t<T>> invalid_values;
};

template <>
const std::string InvalidEnum<Foo>::enum_name{"Foo"};
template <>
const std::vector<uint16_t> InvalidEnum<Foo>::invalid_values{11, 22, 33};

template <>
const std::string InvalidEnum<Bar>::enum_name{"Bar"};
template <>
const std::vector<int64_t> InvalidEnum<Bar>::invalid_values{1, 2, 3};

using MyTypes = testing::Types<Foo, Bar>;
TYPED_TEST_SUITE(InvalidEnum, MyTypes);

TYPED_TEST(InvalidEnum, ThrowsOnInvalidValue) {
  for (auto const &param : TestFixture::invalid_values) {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(param));
    ASSERT_OUTCOME_ERROR(decode<TypeParam>(encoded),
                         DecodeError::INVALID_ENUM_VALUE);
  }
}
