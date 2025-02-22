/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @brief Defines the FromBytes backend for SCALE decoding.
 *
 * This file contains the implementation of the `FromBytes` class,
 * which serves as a backend for SCALE decoding, processing encoded
 * bytes from an input buffer and providing methods to extract data.
 */

#pragma once

#include <cstdint>
#include <ranges>
#include <span>
#include <type_traits>

#include <scale/decoder.hpp>

namespace scale::backend {

  /**
   * @brief Concept to check if a type can be used as a source of bytes.
   *
   * The type must satisfy one of the following conditions:
   * - Be a std::span<const uint8_t> (for contiguous memory like C-arrays or
   * vectors).
   * - Support standard ranges (begin(), end(), and size() methods).
   * - Contain elements convertible to uint8_t.
   *
   * This concept allows FromBytes to work with a wide variety of byte sources,
   * including:
   * - std::vector<uint8_t>
   * - std::list<uint8_t>
   * - std::deque<uint8_t>
   * - std::span<const uint8_t>
   * - C-style arrays (e.g., const uint8_t[])
   * - std::string_view (if containing byte data)
   */
  template <typename T>
  concept ByteSource =
      std::same_as<T, std::span<const uint8_t>> || requires(T source) {
        { std::size(source) } -> std::same_as<size_t>;
        { std::ranges::begin(source) } -> std::input_iterator;
        {
          std::ranges::end(source)
        } -> std::sentinel_for<decltype(std::ranges::begin(source))>;
        { *std::ranges::begin(source) } -> std::convertible_to<uint8_t>;
      };

  /**
   * @class FromBytes
   * @brief Decoder backend that processes bytes from a const reference to an
   * input data source.
   *
   * This class is designed to read bytes from various types of containers,
   * supporting both contiguous and non-contiguous memory layouts. It is
   * optimized for:
   * - Sequential reading of bytes.
   * - Efficient access using std::span for contiguous memory.
   * - Compatibility with C++20 ranges and concepts.
   *
   * The data source type `In` is restricted by the `ByteSource` concept, which
   * ensures that the container:
   * - Supports size(), begin(), and end() methods.
   * - Contains elements convertible to uint8_t.
   * - Can be a std::span<const uint8_t> for contiguous memory like C-arrays or
   * vectors.
   *
   * @tparam In The type of the input data source (default: std::span<const
   * uint8_t>).
   */
  template <typename In = std::span<const uint8_t>>
    requires ByteSource<In>
  class FromBytes final : public Decoder {
   public:
    /**
     * @brief Constructs a FromBytes decoder with a const reference to the input
     * data source.
     *
     * The input data is not copied but referenced, making this constructor
     * efficient for large byte sequences.
     *
     * @param data The input data source to decode from.
     *
     * @note The data source must outlive the FromBytes object to avoid dangling
     * references.
     */
    explicit FromBytes(const In &data) : in_(to_view(data)), it_(in_.begin()) {}

    /**
     * @brief Checks if the data source is a continuous range.
     *
     * This method uses std::ranges::contiguous_range to determine if the
     * underlying data is stored contiguously in memory.
     *
     * @return true if the data source is contiguous, false otherwise.
     */
    [[nodiscard]] constexpr bool isContinuousSource() const override {
      return std::ranges::contiguous_range<decltype(in_)>;
    }

    /**
     * @brief Checks if there are at least `amount` bytes available for reading.
     *
     * This method ensures that there are enough bytes in the buffer to safely
     * perform read operations.
     *
     * @param amount The number of bytes to check.
     * @return true if enough bytes are available, false otherwise.
     */
    [[nodiscard]] bool has(size_t amount) const override {
      return std::distance(it_, in_.end())
             >= static_cast<std::ptrdiff_t>(amount);
    }

    /**
     * @brief Takes and removes the next byte from the buffer.
     *
     * Advances the internal iterator and returns the next byte.
     *
     * @return The next byte as uint8_t.
     * @throws DecodeError::NOT_ENOUGH_DATA if there are no more bytes to read.
     */
    uint8_t take() override {
      if (!has(1)) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      return static_cast<uint8_t>(*it_++);
    }

    /**
     * @brief Reads a sequence of bytes without copying.
     *
     * If the data source is continuous, this method returns a std::span
     * pointing to the next `amount` bytes. Otherwise, it throws a logic_error.
     *
     * @param amount Number of bytes to read.
     * @return A std::span<const uint8_t> containing the requested bytes.
     * @throws DecodeError::NOT_ENOUGH_DATA if not enough bytes are available.
     * @throws std::logic_error if the data source is non-continuous.
     */
    std::span<const uint8_t> read(size_t amount) override {
      if constexpr (std::ranges::contiguous_range<decltype(in_)>) {
        if (!has(amount)) {
          raise(DecodeError::NOT_ENOUGH_DATA);
        }
        auto start = std::distance(in_.begin(), it_);
        auto result = std::span<const uint8_t>(in_.data() + start, amount);
        std::advance(it_, amount);
        return result;
      } else {
        throw std::logic_error("Retrieved span from non-continuous source");
      }
    }

    /**
     * @brief Reads bytes into an output span.
     *
     * This method copies bytes from the internal buffer to the given output
     * span. It uses std::memmove for contiguous sources and std::ranges::copy
     * for non-contiguous ones.
     *
     * @param out The output span to copy the bytes into.
     * @throws DecodeError::NOT_ENOUGH_DATA if not enough bytes are available.
     */
    void read(std::span<uint8_t> out) override {
      if (!has(out.size())) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }

      if constexpr (std::ranges::contiguous_range<decltype(in_)>) {
        auto start = std::distance(in_.begin(), it_);
        std::memmove(out.data(), in_.data() + start, out.size());
      } else {
        std::ranges::copy(it_ | std::views::take(out.size()), out.begin());
      }
      std::advance(it_, out.size());
    }

    /**
     * @brief Returns the number of bytes remaining in the buffer.
     *
     * This method calculates the distance between the current read position
     * and the end of the buffer.
     *
     * @return The number of bytes remaining to be read.
     */
    [[nodiscard]] size_t remaining() const {
      return std::distance(it_, in_.end());
    }

   private:
    /**
     * @brief Converts the input data to a view.
     *
     * - If input is contiguous, returns std::span.
     * - If input is not contiguous, returns std::ranges::subrange.
     *
     * This method abstracts the differences between contiguous and
     * non-contiguous containers, allowing the class to work efficiently with
     * both.
     *
     * @tparam T Type of the input data source.
     * @param data The input data source.
     * @return A view (std::span or std::ranges::subrange) of the input data.
     */
    template <typename T>
    static auto to_view(const T &data) {
      if constexpr (std::ranges::contiguous_range<T>) {
        return std::span<const uint8_t>(data.data(), data.size());
      } else {
        return std::ranges::subrange(data.begin(), data.end());
      }
    }

    decltype(to_view(std::declval<In>())) in_;  ///< Input data view
    decltype(in_.begin()) it_;                  ///< Current read position
  };

}  // namespace scale::backend