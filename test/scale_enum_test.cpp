/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Unit tests for SCALE encoding/decoding and validation of enums.
 */

#include <gtest/gtest.h>
#include <qtils/test/outcome.hpp>
#include <scale/scale.hpp>

using scale::DecodeError;
using scale::impl::memory::decode;
using scale::impl::memory::encode;

/**
 * @brief Typed test fixture for enums with valid values.
 * @tparam T Enum type.
 */
template <typename T>
class ValidEnum : public testing::Test {
 protected:
  const static std::string enum_name;  ///< Name of the enum (for logging)
  const static std::vector<T> values;  ///< List of valid values to test
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

/**
 * @brief Verifies that SCALE decoding of a valid enum restores the original
 * value
 * @given A valid enumeration value
 * @when It is encoded and then decoded using SCALE
 * @then The decoded value must be equal to the original one
 */
TYPED_TEST(ValidEnum, ConsistentEncodingDecoding) {
  SCOPED_TRACE(TestFixture::enum_name);
  for (auto const &param : TestFixture::values) {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(param));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TypeParam>(encoded));
    EXPECT_EQ(decoded, param);
  }
}

/**
 * @brief Ensures SCALE encoding yields raw underlying value
 * @given A valid enumeration value
 * @when It is encoded using SCALE
 * @then The resulting bytes must match the encoding of its underlying value
 */
TYPED_TEST(ValidEnum, CorrectEncoding) {
  using Type = std::underlying_type_t<TypeParam>;
  for (auto const &param : TestFixture::values) {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(param));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<Type>(encoded));
    EXPECT_EQ(decoded, static_cast<Type>(param));
  }
}

/**
 * @brief Typed test fixture for enums with invalid values.
 * @tparam T Enum type.
 */
template <typename T>
class InvalidEnum : public testing::Test {
 protected:
  const static std::string enum_name;  ///< Name of the enum (for logging)
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

/**
 * @brief Ensures decoding fails for invalid enum values
 * @given An invalid underlying value for an enumeration
 * @when It is decoded as that enumeration type
 * @then Decoding must fail with INVALID_ENUM_VALUE error
 */
TYPED_TEST(InvalidEnum, ThrowsOnInvalidValue) {
  for (auto const &param : TestFixture::invalid_values) {
    ASSERT_OUTCOME_SUCCESS(encoded, encode(param));
    ASSERT_OUTCOME_ERROR(decode<TypeParam>(encoded),
                         DecodeError::INVALID_ENUM_VALUE);
  }
}

/**
 * @brief Exhaustively validates values for a given enum type and underlying
 * type.
 * @tparam EnumT The enum type.
 * @tparam UnderlyingT The corresponding integral type.
 * @param valid List of explicitly valid enum values.
 */
template <typename EnumT, typename UnderlyingT>
void validate_enum_range(std::initializer_list<EnumT> valid) {
  for (int i = std::numeric_limits<UnderlyingT>::min();
       i <= std::numeric_limits<UnderlyingT>::max();
       ++i) {
    auto raw = static_cast<UnderlyingT>(i);
    auto val = static_cast<EnumT>(raw);
    bool expected = std::find(valid.begin(), valid.end(), val) != valid.end();
    ASSERT_EQ(scale::is_valid_enum_value<EnumT>(raw), expected)
        << "Failed at raw = " << +raw;
  }
}

/**
 * @brief Tests validation by reflection for signed 1-byte enums
 * @given An int8_t-based enumeration with known valid values
 * @when Each possible raw value is validated using reflection
 * @then Only the predefined enum values must be accepted as valid
 */
TEST(Enum, ValidatingByReflection_I8) {
  enum class Baz : int8_t { A = -10, B = 0, C = 20 };
  validate_enum_range<Baz, int8_t>({Baz::A, Baz::B, Baz::C});
}

/**
 * @brief Tests validation by reflection for unsigned 1-byte enums
 * @given A uint8_t-based enumeration with known valid values
 * @when Each possible raw value is validated using reflection
 * @then Only the predefined enum values must be accepted as valid
 */
TEST(Enum, ValidatingByReflection_U8) {
  enum class Qux : uint8_t { A = 0, B = 10, C = 20 };
  validate_enum_range<Qux, uint8_t>({Qux::A, Qux::B, Qux::C});
}
