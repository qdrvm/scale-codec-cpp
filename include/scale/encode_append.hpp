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
  outcome::result<void> append_or_new_vec(std::vector<uint8_t> &self_encoded,
                                          ConstSpanOfBytes input);
}  // namespace scale
