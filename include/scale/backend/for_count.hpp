/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the ForCount backend for SCALE encoding.
 *
 * This file contains the implementation of the `ForCount` class,
 * which serves as a backend for SCALE encoding, providing a way to
 * count the number of bytes that would be encoded without actually
 * storing them.
 */

#pragma once

#include <scale/encoder_backend.hpp>

namespace scale::backend {

  /**
   * @class ForCount
   * @brief Encoder backend that counts the number of bytes encoded.
   */
  class ForCount final : public EncoderBackend {
   public:
    /**
     * @brief Default constructor.
     */
    ForCount() = default;

    ForCount(ForCount &&) noexcept = delete;
    ForCount(const ForCount &) = delete;
    ForCount &operator=(ForCount &&) noexcept = delete;
    ForCount &operator=(ForCount const &) = delete;

    /**
     * @brief Increments the byte count by one.
     * @param byte The byte to be counted (not stored).
     */
    void put(uint8_t byte) override {
      ++count_;
    }

    /**
     * @brief Increments the byte count by the size of the provided span.
     * @param bytes The span of bytes to be counted (not stored).
     */
    void write(std::span<const uint8_t> bytes) override {
      count_ += bytes.size();
    }

    /**
     * @brief Retrieves the total count of bytes processed.
     * @return The number of bytes counted.
     */
    [[nodiscard]] size_t size() const override {
      return count_;
    }

   private:
    size_t count_{0};  ///< Internal counter tracking the number of bytes.
  };

}  // namespace scale::backend