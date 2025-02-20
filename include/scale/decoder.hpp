/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the Decoder class for SCALE deserialization.
 *
 * This class provides a generic decoding mechanism using a configurable
 * backend. It supports custom configurations when enabled at compile-time.
 */

#pragma once

#include <span>

#include <scale/configurable.hpp>
#include <scale/detail/type_traits.hpp>
#include <scale/outcome/outcome_throw.hpp>
#include <scale/scale_error.hpp>

namespace scale {

  /**
   * @class Decoder
   * @brief A generic SCALE decoder.
   *
   * This class extends Configurable and provides an interface to decode data.
   * It supports custom configurations when enabled at compile-time.
   */
  class Decoder
#ifdef CUSTOM_CONFIG_ENABLED
      : public Configurable
#endif
  {
   public:
    /**
     * @brief Constructs a decoder without custom configurations.
     */
    Decoder() = default;

#ifdef CUSTOM_CONFIG_ENABLED
    /**
     * @brief Constructs a decoder with custom configurations.
     * @param data Immutable span of bytes to decode.
     * @param configs Configuration parameters.
     */
    explicit Decoder(const MaybeConfig auto &...configs)
        : Configurable(configs...) {}
#else
    [[deprecated("Scale has compiled without custom config support")]]  //
    explicit Decoder(const MaybeConfig auto &...configs) = delete;
#endif

    virtual ~Decoder() = default;

    /**
     * @brief Checks whether n more bytes are available.
     * @param amount Number of bytes to check.
     * @return True if amount bytes are available, false otherwise.
     */
    [[nodiscard]] virtual bool has(size_t amount) const = 0;

    /**
     * @brief Takes one byte from source.
     * @return The byte read from source.
     */
    virtual uint8_t take() = 0;

    /**
     * @brief Reads the specified number of bytes and updates the internal
     * buffer.
     * @param amount Number of bytes to read.
     * @return Span of the next required bytes.
     */
    virtual std::span<const uint8_t> read(size_t amount) = 0;
  };

  /**
   * @brief Decodes a value using decoder.
   * @tparam T The type of the value to decode.
   * @param value The value to decode.
   * @return Reference to the decoder for chaining operations.
   */
  template <typename T>
  Decoder &operator>>(Decoder &decoder, T &&value) {
    decode(std::forward<T>(value), decoder);
    return decoder;
  }

}  // namespace scale
