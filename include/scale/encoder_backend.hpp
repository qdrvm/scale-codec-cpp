/**
* Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the base interface for SCALE encoding backends.
 *
 * This file provides an abstract interface for encoding operations in the
 * SCALE serialization format. Implementations of this interface should
 * provide mechanisms to store serialized data efficiently.
 */

#pragma once

#include <cstdint>
#include <span>

namespace scale {

  /**
   * @class EncoderBackend
   * @brief Abstract base class for encoding backends.
   *
   * Implementations of this class provide mechanisms to encode data into a
   * byte-oriented buffer. This interface is designed to be inherited by
   * concrete encoding classes.
   */
  class EncoderBackend
  {
  public:
    /// @brief Virtual destructor for proper cleanup in derived classes.
    virtual ~EncoderBackend() = default;

    /**
     * @brief Stores a single byte into the encoding buffer.
     * @param byte The byte to be stored.
     */
    virtual void put(uint8_t byte) = 0;

    /**
     * @brief Writes a span of bytes into the encoding buffer.
     * @param byte A span containing the bytes to be written.
     */
    virtual void write(std::span<const uint8_t> byte) = 0;

    /**
     * @brief Returns the current size of the encoded data.
     * @return The number of bytes currently stored in the encoding buffer.
     */
    [[nodiscard]] virtual size_t size() const = 0;
  };

} // namespace scale