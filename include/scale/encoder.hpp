/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the Encoder class for SCALE serialization.
 *
 * This class provides a generic encoding mechanism using a configurable
 * backend. It supports custom configuration when enabled at compile-time.
 */

#pragma once

#include <scale/configurable.hpp>
#include <scale/types.hpp>
#include <span>

namespace scale {

  /**
   * @class Encoder
   * @brief A generic SCALE encoder used for byte storage.
   *
   * This class extends Configurable and provides an interface to encode data.
   * It supports custom configurations when enabled at compile-time.
   */
  class Encoder
#ifdef CUSTOM_CONFIG_ENABLED
      : public Configurable
#endif
  {
   public:
    Encoder() = default;

#ifdef CUSTOM_CONFIG_ENABLED
    /**
     * @brief Constructs an encoder with custom configurations.
     * @param configs Configuration parameters.
     */
    explicit Encoder(const MaybeConfig auto &...configs)
        : Configurable(configs...) {}
#else
    /**
     * @brief Constructor is deleted if custom config is not enabled.
     */
    [[deprecated("Scale has compiled without custom config support")]]
    explicit Encoder(const MaybeConfig auto &...configs) = delete;
#endif

    virtual ~Encoder() = default;

    /**
     * @brief Checks if the data receiver is a continuous range.
     *
     * This method uses std::ranges::contiguous_range to determine if the
     * underlying data is stored contiguously in memory.
     *
     * @return true if the data receiver is contiguous, false otherwise.
     */
    [[nodiscard]] virtual constexpr bool isContinuousReceiver() const = 0;

    /**
     * @brief Writes a single byte to the backend.
     * @param byte The byte to be written.
     */
    virtual void put(uint8_t byte) = 0;

    /**
     * @brief Writes a sequence of bytes to the backend.
     * @param bytes A span of bytes to write.
     */
    virtual void write(std::span<const uint8_t> bytes) = 0;

    /**
     * @brief Gets the current size of the encoded data.
     * @return The number of bytes currently stored in the backend.
     */
    [[nodiscard]] virtual size_t size() const = 0;
  };

  /**
   * @brief Encodes a value using an encoder.
   * @tparam T The type of the value to encode.
   * @param encoder The encoder instance.
   * @param value The value to encode.
   * @return Reference to the encoder for chaining operations.
   */
  template <typename T>
  Encoder &operator<<(Encoder &encoder, T &&value) {
    encode(std::forward<T>(value), encoder);
    return encoder;
  }

}  // namespace scale
