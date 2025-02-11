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

  namespace with_buffer {

// #ifdef CUSTOM_CONFIG_ENABLED
//     template <typename T>
//       requires(not ScaleEncoder<T>)
//     outcome::result<ByteArray> encode(const T &v, const auto &config) {
//       Encoder encoder(config);
//       OUTCOME_TRY(encode(v, encoder));
//       return outcome::success(encoder.to_vector());
//     }
//
//     template <typename T>
//       requires(not ScaleDecoder<T>)
//     outcome::result<T> decode(ConstSpanOfBytes bytes, const auto &config) {
//       Decoder decoder(bytes, config);
//       T t;
//       OUTCOME_TRY(decode(t, decoder));
//       return outcome::success(std::move(t));
//     }
// #endif

    template <typename T>
    outcome::result<std::vector<uint8_t>> encode(T &&value) {
      Encoder<backend::ToBytes> encoder;
      try {
        encode(std::forward<T>(value), encoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(encoder).backend().to_vector();
    }

    template <typename T>
    outcome::result<T> decode(ConstSpanOfBytes bytes) {
      Decoder<backend::FromBytes> decoder{bytes};
      T value;
      try {
        decode(value, decoder);
      } catch (std::system_error &e) {
        return outcome::failure(e.code());
      }
      return std::move(value);
    }
  }  // namespace with_buffer

}  // namespace scale::impl
