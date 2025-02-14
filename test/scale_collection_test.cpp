/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <set>

#include <qtils/test/outcome.hpp>
#include <scale/bitvec.hpp>
#include <scale/scale.hpp>

using scale::as_compact;
using scale::BitVec;
using scale::ByteArray;
using scale::CompactInteger;
using scale::DecodeError;
using scale::impl::memory::decode;
using scale::impl::memory::encode;
using Encoder = scale::backend::ToBytes;
using Decoder = scale::backend::FromBytes;

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

    ASSERT_OUTCOME_SUCCESS(out, encode(collection));

    ASSERT_OUTCOME_SUCCESS(match, encode(as_compact(collection.size())));
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

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::vector<bool>>(encoded));
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
  ASSERT_OUTCOME_SUCCESS(match, encode(as_compact(collection.size())));
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(encoded, match));
}

TEST(CollectionTest, encodeBitVec) {
  BitVec collection;
  collection.bits = {
      // clang-format off
      true, true, false, false, false, false, true, false, // 01000011
      false, true, true, false, false                      // ___00110
      // clang-format on
  };
  ByteArray vector_representation = {0b01000011, 0b00110};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));

  ASSERT_OUTCOME_SUCCESS(encodedLen,
                         encode(as_compact(collection.bits.size())));

  auto sizeLen = encodedLen.size();
  auto out =
      std::span(std::next(encoded.data(), sizeLen), encoded.size() - sizeLen);

  ASSERT_TRUE(std::ranges::equal(out, vector_representation));

  ASSERT_OUTCOME_SUCCESS(decoded, decode<BitVec>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded.bits, collection.bits));
}

/**
 * @given collection of items of type uint16_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeCollectionUint16) {
  std::vector<uint16_t> collection = {1, 2, 3, 4};
  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::vector<uint16_t>>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      1, 0,  // first item
      2, 0,  // second item
      3, 0,  // third item
      4, 0,  // fourth item
      // clang-format on
  };
  ASSERT_OUTCOME_SUCCESS(match, encode(as_compact(collection.size())));
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(encoded, match));
}

struct TestStruct : public std::vector<uint16_t> {};

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

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<TestStruct>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      1, 0,  // first item
      2, 0,  // second item
      3, 0,  // third item
      4, 0,  // fourth item
      // clang-format on
  };
  ASSERT_OUTCOME_SUCCESS(match, encode(as_compact(collection.size())));
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(encoded, match));
}

/**
 * @given collection of items of type uint16_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeDequeUint16) {
  std::deque<uint16_t> collection = {1, 2, 3, 4};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::deque<uint16_t>>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      1, 0,  // first item
      2, 0,  // second item
      3, 0,  // third item
      4, 0,  // fourth item
      // clang-format on
  };
  ASSERT_OUTCOME_SUCCESS(match, encode(as_compact(collection.size())));
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(encoded, match));
}

/**
 * @given collection of items of type uint32_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeCollectionUint32) {
  std::vector<uint32_t> collection = {
      0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::deque<uint32_t>>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      0x00, 0x11, 0x22, 0x33, // first item
      0x44, 0x55, 0x66, 0x77,  // second item
      0x88, 0x99, 0xaa, 0xbb,  // third item
      0xcc, 0xdd, 0xee, 0xff,  // fourth item
      // clang-format on
  };
  ASSERT_OUTCOME_SUCCESS(match, encode(as_compact(collection.size())));
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(encoded, match));
}

/**
 * @given collection of items of type uint64_t
 * @when encodeCollection is applied
 * @then expected result is obtained
 */
TEST(CollectionTest, encodeCollectionUint64) {
  std::vector<uint64_t> collection = {0x7766554433221100, 0xffeeddccbbaa9988};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::deque<uint64_t>>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));

  ByteArray data{
      // clang-format off
      0x00, 0x11, 0x22, 0x33,  0x44, 0x55, 0x66, 0x77, // first item
      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,  // second item
      // clang-format on
  };
  ASSERT_OUTCOME_SUCCESS(match, encode(as_compact(collection.size())));
  match.insert(match.end(), data.begin(), data.end());

  ASSERT_TRUE(std::ranges::equal(encoded, match));
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

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));

  Decoder decoder(encoded);
  size_t res;
  ASSERT_NO_THROW(decoder >> as_compact(res));
  ASSERT_EQ(res, length);

  // now only 32768 bytes left in stream
  ASSERT_EQ(decoder.has(length * sizeof(uint16_t)), true);
  ASSERT_EQ(decoder.has(length * sizeof(uint16_t) + 1), false);

  for (auto i = 0; i < length; ++i) {
    uint8_t byte = 0u;
    ASSERT_NO_THROW(decoder >> byte);
    ASSERT_EQ(byte, i % 256);
    ASSERT_NO_THROW(decoder >> byte);
    ASSERT_EQ(byte, 0);
  }

  ASSERT_EQ(decoder.has(1), false);
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

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));

  Decoder decoder(encoded);
  size_t bi;
  ASSERT_NO_THROW(decoder >> as_compact(bi));
  ASSERT_EQ(bi, 1048576);

  // now only 1048576 bytes left in stream
  ASSERT_EQ(decoder.has(1048576), true);
  ASSERT_EQ(decoder.has(1048576 + 1), false);

  for (auto i = 0; i < length; ++i) {
    uint8_t byte{0u};
    ASSERT_NO_THROW((decoder >> byte));
    ASSERT_EQ(byte, i % 256);
  }

  ASSERT_EQ(decoder.has(1), false);
}

/**
 * @given map of <uint32_t, uint32_t>
 * @when encodeCollection is applied
 * @then expected result is obtained: header is 2 byte, items are pairs of 4
 * byte elements each
 */
TEST(CollectionTest, encodeMapTest) {
  std::map<uint32_t, uint32_t> collection = {{1, 5}, {2, 6}, {3, 7}, {4, 8}};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded,
                         (decode<std::map<uint32_t, uint32_t>>(encoded)));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));
}

template <template <typename...> class BaseContainer,
          size_t WithMaxSize,
          typename... Args>
class SizeLimitedContainer : public BaseContainer<Args...> {
  using Base = BaseContainer<Args...>;

 public:
  using Base::Base;
  using typename Base::size_type;

  [[nodiscard]] size_type max_size() const {
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
  std::vector collection{1, 2, 3};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));

  {
    ASSERT_OUTCOME_SUCCESS(decoded,
                           (decode<SizeLimitedVector<4, int>>(encoded)));
    ASSERT_TRUE(std::ranges::equal(decoded, collection));
  }
  {
    ASSERT_OUTCOME_SUCCESS(decoded,
                           (decode<SizeLimitedVector<3, int>>(encoded)));
    ASSERT_TRUE(std::ranges::equal(decoded, collection));
  }
  {
    ASSERT_OUTCOME_ERROR((decode<SizeLimitedVector<2, int>>(encoded)),
                         DecodeError::TOO_MANY_ITEMS);
  }
}

struct ExplicitlyDefinedAsDynamic : std::vector<int> {
  using Collection = std::vector<int>;
  using Collection::begin;
  using Collection::Collection;
  using Collection::end;
  using Collection::insert;
};

TEST(CollectionTest, encodeExplicitlyDefinedAsDynamic) {
  using TestCollection = ExplicitlyDefinedAsDynamic;

  const TestCollection collection{1, 2, 3, 4, 5};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));
}

struct ImplicitlyDefinedAsStatic : std::vector<int> {
  using Collection = std::vector<int>;
  using Collection::Collection;
  ImplicitlyDefinedAsStatic() : Collection(5) {};

 private:
  using std::vector<int>::insert;
  using std::vector<int>::emplace;
};

TEST(CollectionTest, encodeImplicitlyDefinedAsStatic) {
  using TestCollection = ImplicitlyDefinedAsStatic;

  const TestCollection collection{1, 2, 3, 4, 5};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  Decoder decoder(encoded);
  TestCollection decoded{0xff, 0xff, 0xff, 0xff, 0xff};
  decoder >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));
}

struct ImplicitlyDefinedAsDynamic : std::vector<int> {
  using Collection = std::vector<int>;
  using Collection::Collection;
};

TEST(CollectionTest, encodeImplicitlyDefinedAsDynamic) {
  using TestCollection = ImplicitlyDefinedAsDynamic;

  const TestCollection collection{1, 2, 3, 4, 5};

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  Decoder decoder(encoded);
  TestCollection decoded{0xff, 0xff, 0xff};
  decoder >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));
}

struct StaticSpan : std::span<int, 5> {
  using Collection = std::span<int, 5>;
  using Collection::Collection;
};

TEST(CollectionTest, encodeStaticSpan) {
  using TestCollection = StaticSpan;

  std::array<int, 5> original_data{1, 2, 3, 4, 5};
  const TestCollection collection(original_data);

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  Decoder decoder(encoded);
  std::array<int, 5> data{0xff, 0xff, 0xff, 0xff, 0xff};
  TestCollection decoded{data};
  decoder >> decoded;
  ASSERT_TRUE(std::ranges::equal(decoded, collection));
}

TEST(CollectionTest, encodeStringView) {
  using TestCollection = std::string_view;

  std::string original_data = "string";
  const TestCollection collection(original_data);

  ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  ASSERT_OUTCOME_SUCCESS(decoded, decode<std::string>(encoded));
  ASSERT_TRUE(std::ranges::equal(decoded, collection));
}

TEST(CollectionTest, decodeToMutableCollection) {
  {
    using TestCollection = uint16_t[scale::detail::MAX_FIELD_NUM + 1];

    TestCollection collection{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                              12, 13, 14, 15, 16, 17, 18, 19, 20, 21};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    Decoder decoder(encoded);
    TestCollection decoded;
    ASSERT_NO_THROW(decode(decoded, decoder));
    ASSERT_TRUE(std::ranges::equal(decoded, collection));
  }
  {
    using TestCollection =
        std::array<uint16_t, scale::detail::MAX_FIELD_NUM + 1>;

    TestCollection collection{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                              12, 13, 14, 15, 16, 17, 18, 19, 20, 21};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  {
    using TestCollection = std::vector<uint16_t>;

    TestCollection collection{1, 2, 3, 4, 5};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  {
    using TestCollection = std::deque<uint16_t>;

    TestCollection collection{1, 2, 3, 4, 5};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  {
    using TestCollection = std::list<uint16_t>;

    TestCollection collection{1, 2, 3, 4, 5};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  {
    using TestCollection = std::set<uint16_t>;

    TestCollection collection{1, 2, 3, 4, 5};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  {
    using TestCollection = std::map<uint16_t, uint16_t>;

    TestCollection collection{{1, 11}, {2, 22}, {3, 33}};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  {
    using TestCollection = std::unordered_map<uint16_t, uint16_t>;

    TestCollection collection{{1, 11}, {2, 22}, {3, 33}};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
}

TEST(CollectionTest, decodeToImmutableCollection) {
  {
    using TestCollection = const uint16_t[scale::detail::MAX_FIELD_NUM + 1];

    TestCollection collection = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    Decoder decoder(encoded);
    TestCollection decoded{};
    ASSERT_NO_THROW(decode(decoded, decoder));
    ASSERT_TRUE(std::ranges::equal(decoded, collection));
  }
  {
    using TestCollection =
        std::array<const uint16_t, scale::detail::MAX_FIELD_NUM + 1>;

    TestCollection collection{1, 2, 3, 4, 5};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  // Commented because clang warns
  // {
  //   using TestCollection = std::vector<const uint16_t>;
  //
  //   TestCollection collection{1, 2, 3, 4, 5};
  //
  //   ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  //   ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
  //   ASSERT_EQ(decoded, collection);
  // }
  // {
  //   using TestCollection = std::deque<const uint16_t>;
  //
  //   TestCollection collection{1, 2, 3, 4, 5};
  //
  //   ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  //   ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
  //   ASSERT_EQ(decoded, collection);
  // }
  // {
  //   using TestCollection = std::list<const uint16_t>;
  //
  //   TestCollection collection{1, 2, 3, 4, 5};
  //
  //   ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  //   ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
  //   ASSERT_EQ(decoded, collection);
  // }
  // {
  //   using TestCollection = std::set<const uint16_t>;
  //
  //   TestCollection collection{1, 2, 3, 4, 5};
  //
  //   ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
  //   ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
  //   ASSERT_EQ(decoded, collection);
  // }
  {
    using TestCollection = std::map<uint16_t, const uint16_t>;

    TestCollection collection{{1, 11}, {2, 22}, {3, 33}};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
  {
    using TestCollection = std::unordered_map<uint16_t, const uint16_t>;

    TestCollection collection{{1, 11}, {2, 22}, {3, 33}};

    ASSERT_OUTCOME_SUCCESS(encoded, encode(collection));
    ASSERT_OUTCOME_SUCCESS(decoded, decode<TestCollection>(encoded));
    ASSERT_EQ(decoded, collection);
  }
}