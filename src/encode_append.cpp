/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <scale/encode_append.hpp>
#include <scale/scale.hpp>

#include <scale/detail/compact_integer.hpp>

namespace scale {

  outcome::result<void> append_or_new_vec(std::vector<uint8_t> &self_encoded,
                                          ConstSpanOfBytes input) {
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

    // If old and new encoded size length is equal, we don't need to copy the
    // already encoded data.
    if (encoded_old_size_len != encoded_new_size.size()) {
      // reserve place for new size length, old vector and new vector
      self_encoded.reserve(encoded_new_size.size()
                           + (self_encoded.size() - encoded_old_size_len)
                           + opaque_value.v.size());

      // shift the data bytes in a container to give space for the new Compact
      // encoded length prefix
      const auto shift_size = encoded_new_size.size() - encoded_old_size_len;
      self_encoded.resize(self_encoded.size() + shift_size);
      std::memmove(self_encoded.data() + encoded_new_size.size(),
                   self_encoded.data() + encoded_old_size_len,
                   self_encoded.size() - shift_size);
    } else {
      // reserve place for existing and new vector
      self_encoded.reserve(self_encoded.size() + opaque_value.v.size());
    }
    // copy new size bytes
    std::memmove(
        self_encoded.data(), encoded_new_size.data(), encoded_new_size.size());
    // copy new data bytes
    self_encoded.insert(
        self_encoded.end(), opaque_value.v.begin(), opaque_value.v.end());
    return outcome::success();
  }
}  // namespace scale
