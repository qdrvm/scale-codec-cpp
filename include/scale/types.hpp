/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief This file defines the fundamental components of SCALE encoding and
 * decoding.
 *
 * SCALE (Simple Concatenated Aggregate Little-Endian) is a lightweight,
 * efficient, and deterministic serialization format used in blockchain
 * applications. This header provides the primary interfaces and type
 * definitions for encoding and decoding operations.
 */

#pragma once

#include <ranges>
#include <span>
#include <type_traits>
#include <vector>

#include <qtils/tagged.hpp>

namespace scale {

  // Encoding components

  /**
   * @class EncoderBackend
   * @brief Base class for all SCALE encoder backends.
   */
  class EncoderBackend;

  /**
   * @class Encoder
   * @brief Generic SCALE encoder template.
   * @tparam EncoderBackendT The backend type that implements encoding logic.
   *
   * This class is constrained to only accept types derived from EncoderBackend.
   */
  template <typename EncoderBackendT>
    requires std::derived_from<EncoderBackendT, EncoderBackend>
  class Encoder;

  /**
   * @concept ScaleEncoder
   * @brief Concept that defines a valid SCALE encoder.
   *
   * A type satisfies this concept if it defines a `BackendType` and is derived
   * from `Encoder<BackendType>`.
   */
  template <typename T>
  concept ScaleEncoder = requires {
    typename T::BackendType;
    requires std::derived_from<std::remove_cvref_t<T>,
                               Encoder<typename T::BackendType>>;
  };

  // Decoding components

  /**
   * @class DecoderBackend
   * @brief Base class for all SCALE decoder backends.
   */
  class DecoderBackend;

  /**
   * @class Decoder
   * @brief Generic SCALE decoder template.
   * @tparam DecoderBackendT The backend type that implements decoding logic.
   *
   * This class is constrained to only accept types derived from DecoderBackend.
   */
  template <typename DecoderBackendT>
    requires std::derived_from<DecoderBackendT, DecoderBackend>
  class Decoder;

  /**
   * @concept ScaleDecoder
   * @brief Concept that defines a valid SCALE decoder.
   *
   * A type satisfies this concept if it defines a `BackendType` and is derived
   * from `Decoder<BackendType>`.
   */
  template <typename T>
  concept ScaleDecoder = requires {
    typename T::BackendType;
    requires std::derived_from<std::remove_cvref_t<T>,
                               Decoder<typename T::BackendType>>;
  };

  // Types

  /// @brief Convenience alias for arrays of bytes.
  using ByteArray = std::vector<uint8_t>;

  /// @brief Convenience alias for immutable span of bytes.
  using ConstSpanOfBytes = std::span<const uint8_t>;

  /// @brief Convenience alias for mutable span of bytes.
  using MutSpanOfBytes = std::span<uint8_t>;

}  // namespace scale
