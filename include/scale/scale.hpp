/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <qtils/outcome.hpp>

#include <scale/decoder.hpp>
#include <scale/encoder.hpp>

#include <scale/backend/for_count.hpp>
#include <scale/backend/from_bytes.hpp>
#include <scale/backend/to_bytes.hpp>

#include <scale/detail/collections.hpp>
#include <scale/detail/compact_integer.hpp>
#include <scale/detail/decomposable.hpp>
#include <scale/detail/enum.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#include <scale/detail/optional.hpp>
#include <scale/detail/smart_pointers.hpp>
#include <scale/detail/variant.hpp>

namespace scale::impl {

  // Well done implementation using outcome::result and data in memory
  namespace memory {
    using Encoder = Encoder<backend::ToBytes>;

    template <typename T>
    outcome::result<std::vector<uint8_t>> encode(T &&value) {
      Encoder encoder;
      try {
        encode(std::forward<T>(value), encoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(encoder).backend().to_vector();
    }

    using Decoder = Decoder<backend::FromBytes>;

    template <typename T>
    outcome::result<T> decode(ConstSpanOfBytes bytes) {
      Decoder decoder{bytes};
      T value;
      try {
        decode(value, decoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(value);
    }
  }  // namespace memory

}  // namespace scale::impl
