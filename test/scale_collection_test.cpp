/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <scale/scale.hpp>

using scale::BitVec;
using scale::ByteArray;
using scale::CompactInteger;
using scale::decode;
using scale::DecodeError;
using scale::encode;
using scale::Length;
using scale::ScaleDecoderStream;
using scale::ScaleEncoderStream;

auto encodeLen = [](size_t i) {
  ScaleEncoderStream s;
#ifdef JAM_COMPATIBILITY_ENABLED
  scale::detail::encodeJamCompactInteger(i, s);
#else
  scale::detail::encodeCompactInteger(i, s);
#endif
  return s.to_vector();
};

/**
 * @given collection of 80 items of type uint8_t
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are 1 byte each
 */
TEST(CollectionTest, encodeCollectionOf80) {
  for (size_t length = 60; length <= 130; ++length) {
    ByteArray collection;
    collection.reserve(length);
    for (auto i = 0; i < length; ++i) {
      collection.push_back(i % 256);
    }
    ScaleEncoderStream s;
    ASSERT_NO_THROW((s << collection));
    auto &&out = s.to_vector();

    auto match = encodeLen(collection.size());  // header
    match.insert(match.end(), collection.begin(), collection.end());

    ASSERT_EQ(out, match);
  }
}

/**
 * @given vector of bools
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are 1 byte each
 */
TEST(CollectionTest, encodeVectorOfBool) {
  std::vector<bool> collection = {true, false, true, false, false, false};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::vector<bool> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      1,  // first item
      0,  // second item
      1,  // third item
      0,  // fourth item
      0,  // fifth item
      0,  // sixths item
      // clang-format on
  };
  auto match = encodeLen(collection.size());
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(out, match));
}

TEST(CollectionTest, encodeBitVec) {
  auto collection = BitVec{{
      // clang-format off
      true, true, false, false, false, false, true, false, // 01000011
      false, true, true, false, false                      // ___00110
      // clang-format on
  }};
  ByteArray vector_representation = {0b01000011, 0b00110};

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&encoded = s.to_vector();

  auto sizeLen = encodeLen(collection.bits.size()).size();
  auto out =
      std::span(std::next(encoded.data(), sizeLen), encoded.size() - sizeLen);

  ASSERT_TRUE(std::ranges::equal(out, vector_representation));

  auto stream = ScaleDecoderStream(encoded);
  BitVec decoded;
  stream >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded.bits, collection.bits));
}

/**
 * @given collection of items of type uint16_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeCollectionUint16) {
  std::vector<uint16_t> collection = {1, 2, 3, 4};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::vector<uint16_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      1, 0,  // first item
      2, 0,  // second item
      3, 0,  // third item
      4, 0,  // fourth item
      // clang-format on
  };
  auto match = encodeLen(collection.size());
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(out, match));
}

struct TestStruct : public std::vector<uint16_t> {
  // friend ScaleEncoderStream &operator<<(ScaleEncoderStream &s,
  //                                       const TestStruct &test_struct) {
  //   return s << static_cast<const std::vector<uint16_t> &>(test_struct);
  // }
  // friend ScaleDecoderStream &operator>>(ScaleDecoderStream &s,
  //                                       TestStruct &test_struct) {
  //   return s >> static_cast<std::vector<uint16_t> &>(test_struct);
  // }
};

/**
 * @given collection of items of type uint16_t, derived from std::vector
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeDerivedCollectionUint16) {
  TestStruct collection;
  collection.push_back(1);
  collection.push_back(2);
  collection.push_back(3);
  collection.push_back(4);

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestStruct decoded;
  stream >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      1, 0,  // first item
      2, 0,  // second item
      3, 0,  // third item
      4, 0,  // fourth item
      // clang-format on
  };
  auto match = encodeLen(collection.size());
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(out, match));
}

/**
 * @given collection of items of type uint16_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeDequeUint16) {
  std::deque<uint16_t> collection = {1, 2, 3, 4};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::deque<uint16_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      1, 0,  // first item
      2, 0,  // second item
      3, 0,  // third item
      4, 0,  // fourth item
      // clang-format on
  };
  auto match = encodeLen(collection.size());
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(out, match));
}

/**
 * @given collection of items of type uint32_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeCollectionUint32) {
  std::vector<uint32_t> collection = {
      0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::deque<uint32_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      0x00, 0x11, 0x22, 0x33, // first item
      0x44, 0x55, 0x66, 0x77,  // second item
      0x88, 0x99, 0xaa, 0xbb,  // third item
      0xcc, 0xdd, 0xee, 0xff,  // fourth item
      // clang-format on
  };
  auto match = encodeLen(collection.size());
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(out, match));
}

/**
 * @given collection of items of type uint64_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeCollectionUint64) {
  std::vector<uint64_t> collection = {0x7766554433221100, 0xffeeddccbbaa9988};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::deque<uint64_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      0x00, 0x11, 0x22, 0x33,  0x44, 0x55, 0x66, 0x77, // first item
      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,  // second item
      // clang-format on
  };
  auto match = encodeLen(collection.size());
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(out, match));
}

/**
 * @given collection of items of type uint16_t containing 2^14 items
 * where collection[i]  == i % 256
 * @when encodeCollection is applied
 * @then obtain byte array of length 32772 bytes
 * where each second byte == 0 and collection[(i-4)/2] == (i/2) % 256
 */
TEST(CollectionTest, encodeLongCollectionUint16) {
  std::vector<uint16_t> collection;
  auto length = 16384;
  collection.reserve(length);
  for (auto i = 0; i < length; ++i) {
    collection.push_back(i % 256);
  }

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  Length res{};
  ASSERT_NO_THROW(stream >> res);
  ASSERT_EQ(res, length);

  // now only 32768 bytes left in stream
  ASSERT_EQ(stream.hasMore(length * sizeof(uint16_t)), true);
  ASSERT_EQ(stream.hasMore(length * sizeof(uint16_t) + 1), false);

  for (auto i = 0; i < length; ++i) {
    uint8_t byte = 0u;
    ASSERT_NO_THROW(stream >> byte);
    ASSERT_EQ(byte, i % 256);
    ASSERT_NO_THROW(stream >> byte);
    ASSERT_EQ(byte, 0);
  }

  ASSERT_EQ(stream.hasMore(1), false);
}

/**
 * @given very long collection of items of type uint8_t containing 2^20 items
 * this number takes ~ 1 Mb of data
 * where collection[i]  == i % 256
 * @when encodeCollection is applied
 * @then obtain byte array of length 1048576 + 4 bytes (header) bytes
 * where first bytes repreent header, other are data itself
 * where each byte after header == i%256
 */

TEST(CollectionTest, encodeVeryLongCollectionUint8) {
  auto length = 1048576;  // 2^20
  ByteArray collection;
  collection.reserve(length);
  for (auto i = 0; i < length; ++i) {
    collection.push_back(i % 256);
  }

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  Length bi{};
  ASSERT_NO_THROW(stream >> bi);
  ASSERT_EQ(bi, 1048576);

  // now only 1048576 bytes left in stream
  ASSERT_EQ(stream.hasMore(1048576), true);
  ASSERT_EQ(stream.hasMore(1048576 + 1), false);

  for (auto i = 0; i < length; ++i) {
    uint8_t byte{0u};
    ASSERT_NO_THROW((stream >> byte));
    ASSERT_EQ(byte, i % 256);
  }

  ASSERT_EQ(stream.hasMore(1), false);
}

/**
 * @given map of <uint32_t, uint32_t>
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are pairs of 4
 * byte elements each
 */
TEST(CollectionTest, encodeMapTest) {
  std::map<uint32_t, uint32_t> collection = {{1, 5}, {2, 6}, {3, 7}, {4, 8}};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::map<uint32_t, uint32_t> decoded;
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

template <template <typename...> class BaseContainer,
          size_t WithMaxSize,
          typename... Args>
class SizeLimitedContainer : public BaseContainer<Args...> {
  using Base = BaseContainer<Args...>;

 public:
  using Base::Base;
  using typename Base::size_type;

  size_type max_size() const {
    return WithMaxSize;
  }
};

template <size_t WithMaxSize, typename... Args>
using SizeLimitedVector =
    SizeLimitedContainer<std::vector, WithMaxSize, Args...>;

/**
 * @given encoded 3-elements collection
 * @when decode it to collection limited by size 4, 3 and 2 max size
 * @then if max_size is enough, it is done successful, and error otherwise
 */
TEST(CollectionTest, decodeSizeLimitedCollection) {
  std::vector<int> collection{1, 2, 3};

  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  {
    auto stream = ScaleDecoderStream(out);
    SizeLimitedVector<4, int> decoded;
    ASSERT_NO_THROW((stream >> decoded));
    ASSERT_TRUE(std::equal(
        decoded.begin(), decoded.end(), collection.begin(), collection.end()));
  }
  {
    auto stream = ScaleDecoderStream(out);
    SizeLimitedVector<3, int> decoded;
    ASSERT_NO_THROW((stream >> decoded));
    ASSERT_TRUE(std::equal(
        decoded.begin(), decoded.end(), collection.begin(), collection.end()));
  }
  {
    auto stream = ScaleDecoderStream(out);
    SizeLimitedVector<2, int> decoded;

    try {
      stream >> decoded;
      FAIL() << "Exception expected";
    } catch (std::system_error &e) {
      EXPECT_EQ(e.code(), DecodeError::TOO_MANY_ITEMS);
    }
  }
}

struct ExplicitlyDefinedAsDynamic : public std::vector<int> {
  using Collection = std::vector<int>;
  using Collection::Collection;
};

TEST(CollectionTest, encodeExplicitlyDefinedAsDynamic) {
  using TestCollection = ExplicitlyDefinedAsDynamic;

  const TestCollection collection{1, 2, 3, 4, 5};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestCollection decoded{0xff, 0xff, 0xff};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

struct ImplicitlyDefinedAsStatic : public std::vector<int> {
  using Collection = std::vector<int>;
  using Collection::Collection;

 private:
  using std::vector<int>::insert;
  using std::vector<int>::emplace;
};

TEST(CollectionTest, encodeImplicitlyDefinedAsStatic) {
  using TestCollection = ImplicitlyDefinedAsStatic;

  const TestCollection collection{1, 2, 3, 4, 5};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestCollection decoded{0xff, 0xff, 0xff, 0xff, 0xff};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

struct ImplicitlyDefinedAsDynamic : public std::vector<int> {
  using Collection = std::vector<int>;
  using Collection::Collection;
};

TEST(CollectionTest, encodeImplicitlyDefinedAsDynamic) {
  using TestCollection = ImplicitlyDefinedAsDynamic;

  const TestCollection collection{1, 2, 3, 4, 5};
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  TestCollection decoded{0xff, 0xff, 0xff};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}

struct StaticSpan : public std::span<int, 5> {
  using Collection = std::span<int, 5>;
  using Collection::Collection;
};

TEST(CollectionTest, encodeStaticSpan) {
  using TestCollection = StaticSpan;

  std::array<int, 5> original_data{1, 2, 3, 4, 5};
  const TestCollection collection(original_data);
  ScaleEncoderStream s;
  ASSERT_NO_THROW((s << collection));
  auto &&out = s.to_vector();

  auto stream = ScaleDecoderStream(out);
  std::array<int, 5> data{0xff, 0xff, 0xff, 0xff, 0xff};
  TestCollection decoded{data};
  stream >> decoded;
  ASSERT_TRUE(std::equal(
      decoded.begin(), decoded.end(), collection.begin(), collection.end()));
}
