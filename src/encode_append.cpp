/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <scale/encode_append.hpp>
#include <scale/scale.hpp>

#include <scale/detail/compact_integer.hpp>

namespace scale {

  /**
   * @param data Scale encoded vector of EncodeOpaqueValues
   * @return tuple containing:
   *  1. new length of vector after inserting there one more EncodeOpaqueValues
   *  2. current length of CompactInteger-scale-encoded length of
   * EncodeOpaqueValues vector
   *  3. length of CompactInteger-scale-encoded length of EncodeOpaqueValues
   * vector after insertion there one more EncodeOpaqueValue
   */
  outcome::result<std::tuple<uint32_t, uint32_t, uint32_t>> extract_length_data(
      const std::vector<uint8_t> &data) {
    OUTCOME_TRY(len, impl::memory::decode<Compact<uint32_t>>(data));
    auto old_len = untagged(len);
    auto new_len = old_len + 1;
    auto encoded_len = lengthOfEncodedCompactInteger(old_len);
    auto encoded_new_len = lengthOfEncodedCompactInteger(new_len);
    return std::make_tuple(new_len, encoded_len, encoded_new_len);
  }

  outcome::result<void> append_or_new_vec(std::vector<uint8_t> &self_encoded,
                                          ConstSpanOfBytes input) {
    EncodeOpaqueValue opaque_value{.v = input};

    // No data present, just encode the given input data.
    if (self_encoded.empty()) {
      backend::ToBytes encoder;
      encoder << std::vector{opaque_value};
      self_encoded = encoder.to_vector();
      return outcome::success();
    }

    OUTCOME_TRY(extract_tuple, extract_length_data(self_encoded));
    const auto &[new_len, encoded_len, encoded_new_len] = extract_tuple;

    auto replace_len = [new_len = new_len](std::vector<uint8_t> &dest) {
      auto e = impl::memory::encode(as_compact(new_len)).value();
      std::move(e.begin(), e.end(), dest.begin());
    };

    // If old and new encoded len is equal, we don't need to copy the
    // already encoded data.
    if (encoded_len != encoded_new_len) {
      // reserve place for new len, old vector and new vector
      self_encoded.reserve(encoded_new_len + (self_encoded.size() - encoded_len)
                           + opaque_value.v.size());

      // shift the data bytes in a container to give space for the new Compact
      // encoded length prefix
      const auto shift_size = encoded_new_len - encoded_len;
      self_encoded.resize(self_encoded.size() + shift_size);
      std::rotate(self_encoded.rbegin(),
                  self_encoded.rbegin() + shift_size,
                  self_encoded.rend());
    } else {
      // reserve place for existing and new vector
      self_encoded.reserve(self_encoded.size() + opaque_value.v.size());
    }
    replace_len(self_encoded);
    self_encoded.insert(
        self_encoded.end(), opaque_value.v.begin(), opaque_value.v.end());
    return outcome::success();
  }
}  // namespace scale
