/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/outcome.hpp>
#include <scale/scale.hpp>

namespace scale {

  /**
   * Vector wrapper, that is scale encoded without prepended CompactInteger
   */
  struct EncodeOpaqueValue {
    ConstSpanOfBytes v;

    friend void encode(const EncodeOpaqueValue &opaque, Encoder &encoder) {
      for (auto &item : opaque.v) {
        encode(item, encoder);
      }
    }
  };

  /**
   * Adds an EncodeOpaqueValue to a scale encoded vector of EncodeOpaqueValue's.
   * If the current vector is empty, then it is replaced by a new
   * EncodeOpaqueValue
   * In other words, what actually happens could be implemented like that:
   * @code{.cpp}
   * auto vec = scale::decode<vector<EncodeOpaqueValue>>(self_encoded);
   * vec.push_back(scale::encode(EncodeOpaqueValue(input));
   * self_encoded = scale::encode(vec);
   * @endcode
   * but the actual implementation is a bit more optimal
   * @param self_encoded - An encoded vector of EncodeOpaqueValue
   * @param input - A vector encoded as an EncodeOpaqueValue and added to
   * \param self_encoded
   * @return success if input was appended to self_encoded, failure otherwise
   */
  inline outcome::result<void> append_or_new_vec(
      std::vector<uint8_t> &self_encoded, ConstSpanOfBytes input) {
    EncodeOpaqueValue opaque_value{.v = input};

    // No data present, just encode the given input data.
    if (self_encoded.empty()) {
      backend::ToBytes encoder(self_encoded);
      encode(std::vector{opaque_value}, encoder);
      return outcome::success();
    }

    // Take old size, calculate old size length and encode new size
    OUTCOME_TRY(size, impl::memory::decode<Compact<uint32_t>>(self_encoded));
    auto old_size = untagged(size);
    auto new_size = old_size + 1;
    auto encoded_old_size_len = lengthOfEncodedCompactInteger(old_size);
    OUTCOME_TRY(encoded_new_size, impl::memory::encode(as_compact(new_size)));

    const auto old_data_size = self_encoded.size();
    const auto encoded_new_size_len = encoded_new_size.size();
    const auto shift_size = encoded_new_size_len - encoded_old_size_len;

    // if old and new encoded size length is equal, no need to shift data
    if (encoded_old_size_len != encoded_new_size_len) {
      // reserve place for new size length, old vector and new vector
      self_encoded.reserve(old_data_size + shift_size + opaque_value.v.size());

      // increase size to make space for new size encoding
      self_encoded.resize(old_data_size + shift_size);

      // shift existing data
      std::memmove(self_encoded.data() + encoded_new_size_len,
                   self_encoded.data() + encoded_old_size_len,
                   old_data_size - encoded_old_size_len);
    } else {
      // reserve place for existing and new vector
      self_encoded.reserve(old_data_size + opaque_value.v.size());
    }

    // copy new size bytes at the beginning
    std::memmove(
        self_encoded.data(), encoded_new_size.data(), encoded_new_size.size());
    // append new data bytes
    self_encoded.insert(
        self_encoded.end(), opaque_value.v.begin(), opaque_value.v.end());
    return outcome::success();
  }

}  // namespace scale
