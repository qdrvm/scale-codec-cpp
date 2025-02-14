/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Provides encoding and decoding utilities for collections
 *        in SCALE serialization.
 *
 * This file defines functions to handle encoding and decoding
 * of different types of collections, including static, dynamic,
 * resizable, and extensible collections.
 */

#pragma once

#include <scale/detail/collections_type_traits.hpp>
#include <scale/detail/decomposable_type_traits.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#include <scale/detail/tagged.hpp>
#include <scale/types.hpp>

namespace scale {

  using detail::collections::DynamicCollection;
  using detail::collections::DynamicSpan;
  using detail::collections::ExtensibleBackCollection;
  using detail::collections::RandomExtensibleCollection;
  using detail::collections::ResizeableCollection;
  using detail::collections::StaticCollection;
  using detail::decomposable::Decomposable;
  using detail::decomposable::DecomposableArray;

  /**
   * @brief Encodes a dynamic collection using SCALE encoding.
   * @param collection The collection to encode.
   * @param encoder The encoder instance to write to.
   */
  void encode(DynamicCollection auto &&collection, Encoder &encoder)
    requires NoTagged<decltype(collection)>
             and (not std::same_as<std::remove_cvref_t<decltype(collection)>,
                                   std::vector<bool>>)
  {
    encode(as_compact(collection.size()), encoder);
    for (auto &&item : std::forward<decltype(collection)>(collection)) {
      encode(item, encoder);
    }
  }

  /**
   * @brief Encodes a static collection using SCALE encoding.
   * @param collection The collection to encode.
   * @param encoder The encoder instance to write to.
   */
  void encode(StaticCollection auto &&collection, Encoder &encoder)
    requires NoTagged<decltype(collection)>
             and (not DecomposableArray<decltype(collection)>)
  {
    for (auto &&item : std::forward<decltype(collection)>(collection)) {
      encode(item, encoder);
    }
  }

  /**
   * @brief Encodes a string view using SCALE encoding.
   * @param view The string view to encode.
   * @param encoder The encoder instance to write to.
   */
  inline void encode(const std::string_view view, Encoder &encoder) {
    encode(as_compact(view.size()), encoder);
    encoder.write(
        {reinterpret_cast<const uint8_t *>(view.data()), view.size()});
  }

  /**
   * @brief Encodes a vector of boolean values using SCALE encoding.
   * @param vector The boolean vector to encode.
   * @param encoder The encoder instance to write to.
   */
  template <typename T>
    requires std::same_as<std::remove_cvref_t<T>, std::vector<bool>>
  void encode(T &&vector, Encoder &encoder)
    requires NoTagged<decltype(vector)>
  {
    encode(as_compact(vector.size()), encoder);
    for (const bool item : vector) {
      encoder.put(static_cast<uint8_t>(item ? 1 : 0));
    }
  }

  /**
   * @brief Decoding for dynamic spans is prohibited as potentially dangerous.
   */
  void decode(DynamicSpan auto &collection, Decoder &decoder) = delete;

  /**
   * @brief Decodes a static collection using SCALE decoding.
   * @param collection The collection to decode.
   * @param decoder The decoder instance to read from.
   */
  void decode(StaticCollection auto &collection, Decoder &decoder)
    requires NoTagged<decltype(collection)>
             and (not Decomposable<decltype(collection)>)
  {
    using Collection = std::remove_cvref_t<decltype(collection)>;
    using MutableCollection = std::conditional_t<
        detail::collections::ImmutableCollection<Collection>,
        detail::collections::mutable_collection_t<Collection>,
        Collection>;

    auto &mutable_collection = reinterpret_cast<MutableCollection &>(
        const_cast<Collection &>(collection));
    for (decltype(auto) item : mutable_collection) {
      decode(item, decoder);
    }
  }

  /**
   * @brief Decodes an extensible back collection using SCALE decoding.
   * @param collection The collection to decode.
   * @param decoder The decoder instance to read from.
   */
  void decode(ExtensibleBackCollection auto &collection, Decoder &decoder)
    requires NoTagged<decltype(collection)>
  {
    using size_type = typename std::decay_t<decltype(collection)>::size_type;

    size_t item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    collection.clear();
    try {
      collection.reserve(item_count);
    } catch (const std::bad_alloc &) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    for (size_type i = 0u; i < item_count; ++i) {
      collection.emplace_back();
      decode(collection.back(), decoder);
    }
  }

  /**
   * @brief Decodes a resizable collection using SCALE decoding.
   * @param collection The collection to decode.
   * @param decoder The decoder instance to read from.
   */
  void decode(ResizeableCollection auto &collection, Decoder &decoder)
    requires NoTagged<decltype(collection)>
  {
    using Collection = std::remove_cvref_t<decltype(collection)>;
    using MutableCollection = std::conditional_t<
        detail::collections::ImmutableCollection<Collection>,
        detail::collections::mutable_collection_t<Collection>,
        Collection>;

    auto &mutable_collection =
        reinterpret_cast<MutableCollection &>(collection);

    size_t item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    try {
      mutable_collection.resize(item_count);
    } catch (const std::bad_alloc &) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    for (decltype(auto) item : mutable_collection) {
      decode(item, decoder);
    }
  }

  /**
   * @brief Decodes a random extensible collection using SCALE decoding.
   * @note non-sequential collection, which can not be reserved space or resize,
   * but each element can be emplaced while decoding
   */
  void decode(RandomExtensibleCollection auto &collection, Decoder &decoder)
    requires NoTagged<decltype(collection)>
  {
    using size_type = typename std::decay_t<decltype(collection)>::size_type;
    using value_type = std::remove_const_t<
        typename std::decay_t<decltype(collection)>::value_type>;

    size_type item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    collection.clear();
    for (size_type i = 0u; i < item_count; ++i) {
      value_type item{};
      decode(item, decoder);
      try {
        collection.emplace(std::move(item));
      } catch (const std::bad_alloc &) {
        raise(DecodeError::TOO_MANY_ITEMS);
      }
    }
  }

  /**
   * @brief Decodes a vector of booleans using SCALE decoding.
   * @param collection The collection to decode.
   * @param decoder The decoder instance to read from.
   */
  inline void decode(std::vector<bool> &collection, Decoder &decoder) {
    size_t item_count;
    decode(as_compact(item_count), decoder);
    if (item_count > collection.max_size()) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    try {
      collection.resize(item_count);
    } catch (const std::bad_alloc &) {
      raise(DecodeError::TOO_MANY_ITEMS);
    }

    for (size_t i = 0u; i < item_count; ++i) {
      bool item;
      decode(item, decoder);
      collection[i] = item;
    }
  }

}  // namespace scale
