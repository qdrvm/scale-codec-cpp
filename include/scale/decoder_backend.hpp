/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the base interface for SCALE decoding backends.
 *
 * This file provides an abstract interface for decoding operations in the
 * SCALE serialization format. Implementations of this interface should
 * provide mechanisms to read serialized data efficiently.
 */

#pragma once

#include <cstdint>
#include <span>

namespace scale {

  /**
   * @class DecoderBackend
   * @brief Abstract base class for decoding backends.
   *
   * Implementations of this class provide mechanisms to decode data from a
   * byte-oriented buffer. This interface is designed to be inherited by
   * concrete decoding classes.
   */
  class DecoderBackend {
   public:
    /// @brief Virtual destructor for proper cleanup in derived classes.
    virtual ~DecoderBackend() = default;

    /**
     * @brief Checks if the decoder has the specified amount of data available.
     * @param amount The number of bytes to check for.
     * @return True if the specified amount of data is available, false
     * otherwise.
     */
    [[nodiscard]] virtual bool has(size_t amount) const = 0;

    /**
     * @brief Takes and returns a single byte from the backend.
     * @return The byte read from the backend.
     */
    virtual uint8_t take() = 0;

    /**
     * @brief Reads a span of bytes into the provided output buffer.
     * @param out The span where the read bytes will be stored.
     */
    virtual void read(std::span<uint8_t> out) = 0;
  };

}  // namespace scale
