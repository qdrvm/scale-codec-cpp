/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#define CFG_TESTING

#include <gtest/gtest.h>

#include <set>

#include <scale/bit_vector.hpp>
#include <scale/scale.hpp>

using scale::as_compact;
using scale::BitVector;
using scale::ByteArray;
using scale::CompactInteger;
using scale::DecodeError;
using scale::SmallBitVector;
using scale::impl::memory::decode;
using scale::impl::memory::encode;
using Encoder = scale::backend::ToBytes<>;
using Decoder = scale::backend::FromBytes<>;

TEST(BitVector, SmallBitVector) {
  using SBV = SmallBitVector<>;
  SBV collection;
  ASSERT_EQ(collection.size(), 0);
  ASSERT_EQ(collection.data(), 0);

  for (auto i = 0; i < SBV::data_bits; ++i) {
    ASSERT_THROW(collection.at(i), std::out_of_range);
    collection.push_back(false);
    ASSERT_EQ(collection.size(), i + 1);
    ASSERT_EQ(collection.data(), 0);
    ASSERT_NO_THROW(collection.at(i));
    ASSERT_FALSE(collection[i]);
    ASSERT_EQ(collection[i], collection.at(i));
    ASSERT_THROW(collection.at(i + 1), std::out_of_range);
  }
  ASSERT_THROW(collection.push_back(false), std::overflow_error);

  collection.resize(0);
  ASSERT_EQ(collection.size(), 0);
  ASSERT_EQ(collection.data(), 0);
  for (auto i = 0; i < SBV::data_bits; ++i) {
    ASSERT_THROW(collection.at(i), std::out_of_range);
    collection.push_back(true);
    ASSERT_EQ(collection.size(), i + 1);
    ASSERT_EQ(collection.data(), (1ull << (i + 1)) - 1);
    ASSERT_NO_THROW(collection.at(i));
    ASSERT_TRUE(collection[i]);
    ASSERT_EQ(collection[i], collection.at(i));
    ASSERT_THROW(collection.at(i + 1), std::out_of_range);
  }
  ASSERT_THROW(collection.push_back(false), std::overflow_error);

  for (auto ri = 0; ri < SBV::data_bits; ++ri) {
    auto size = collection.size();
    ASSERT_EQ(size, SBV::data_bits - ri);
    ASSERT_THROW(collection.at(size), std::out_of_range);
    auto data = collection.data();

    auto new_size = size - 1;
    collection.resize(new_size);
    ASSERT_EQ(collection.size(), new_size);
    ASSERT_EQ(collection.data(), data >> 1);

    ASSERT_THROW(collection.at(new_size), std::out_of_range);
    if (new_size > 0) {
      ASSERT_NO_THROW(collection.at(new_size - 1));
      ASSERT_TRUE(collection[new_size - 1]);
      ASSERT_EQ(collection[new_size - 1], collection.at(new_size - 1));
    }
  }

  for (auto i = 0; i < SBV::data_bits; ++i) {
    collection.push_back(i % 2 > 0);
  }
  ASSERT_EQ(collection.size(), SBV::data_bits);
  ASSERT_EQ(collection.size(), collection.capacity());
  ASSERT_EQ(collection.capacity(), SBV::data_bits);
  collection.clear();
  ASSERT_EQ(collection.size(), 0);
  ASSERT_EQ(collection.data(), 0);
  ASSERT_EQ(collection.capacity(), SBV::data_bits);
}

TEST(BitVector, BitVector) {
  using BV = BitVector;
  BV collection;
  ASSERT_EQ(collection.size(), 0);
  ASSERT_EQ(collection.bytes().size(), 0);

  for (auto target_size : {100, 500}) {
    std::cout << "Target size: " << target_size << '\n';

    for (auto i = 0; i < target_size; ++i) {
      collection.push_back(i % 2 > 0);
    }

    collection.clear();
    ASSERT_EQ(collection.size(), 0);

    std::cout << "Fill by 'false':\n";
    for (auto i = 0; i < target_size; ++i) {
      std::cout << ' ' << i << '\n';
      ASSERT_THROW(collection.at(i), std::out_of_range);
      collection.push_back(false);
      ASSERT_EQ(collection.size(), i + 1);
      ASSERT_EQ(collection.bytes().size(), (collection.size() + 7) / CHAR_BIT);

      auto size_in_bytes = (collection.size() + 7) / 8;
      ASSERT_EQ(size_in_bytes, collection.bytes().size());
      ASSERT_TRUE(std::ranges::all_of(collection.bytes(),
                                      [](uint8_t x) { return x == 0x00; }));

      ASSERT_NO_THROW(collection.at(i));
      ASSERT_FALSE(collection[i]);
      EXPECT_EQ(collection[i], collection.back());
      EXPECT_EQ(collection[i], collection.at(i));
      ASSERT_THROW(collection.at(i + 1), std::out_of_range);
    }

    std::cout << "Resize to 0\n";
    collection.resize(0);
    ASSERT_EQ(collection.size(), 0);
    ASSERT_EQ(collection.bytes().size(), 0);

    std::cout << "Fill by 'true':\n";
    for (auto i = 0; i < target_size; ++i) {
      std::cout << ' ' << i << '\n';
      ASSERT_THROW(collection.at(i), std::out_of_range);
      collection.push_back(true);
      ASSERT_EQ(collection.size(), i + 1);
      ASSERT_EQ(collection.bytes().size(), (collection.size() + 7) / CHAR_BIT);

      auto byte = i / 8;
      auto bit = i % 8;
      EXPECT_LT(byte, collection.bytes().size());
      EXPECT_TRUE(std::ranges::all_of(
          collection.bytes().first(collection.bytes().size() - 1),
          [](const uint8_t x) { return x == 0xff; }));

      EXPECT_EQ(collection.bytes()[byte],
                static_cast<uint8_t>(-1) >> (CHAR_BIT - 1 - bit));

      ASSERT_NO_THROW(collection.at(i));
      ASSERT_TRUE(collection[i]);
      ASSERT_EQ(collection[i], collection.back());
      ASSERT_THROW(collection.at(i + 1), std::out_of_range);
    }

    std::cout << "Resize by drop of last:\n";
    for (auto ri = 0; ri < target_size; ++ri) {
      std::cout << ' ' << (collection.size() - 1) << '\n';
      auto size = collection.size();
      ASSERT_EQ(size, target_size - ri);
      ASSERT_THROW(collection.at(size), std::out_of_range);
      auto new_size = size - 1;
      collection.resize(new_size);
      ASSERT_EQ(collection.size(), new_size);

      ASSERT_THROW(collection.at(new_size), std::out_of_range);
      if (new_size > 0) {
        ASSERT_NO_THROW(collection.at(new_size - 1));
        ASSERT_TRUE(collection[new_size - 1]);
      }
    }

    for (auto i = 0; i < target_size; ++i) {
      collection.push_back(i % 2 > 0);
    }
    ASSERT_EQ(collection.size(), target_size);
    collection.clear();
    ASSERT_EQ(collection.size(), 0);
  }
}