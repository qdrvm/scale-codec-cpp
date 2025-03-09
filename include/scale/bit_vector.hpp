/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <array>
#include <limits>
#include <vector>

#include <scale/detail/compact_integer.hpp>
#include <scale/detail/decomposable_type_traits.hpp>
#include <scale/types.hpp>

namespace scale {

  /**
   * @class SmallBitVector
   * @brief Defines structure for encoding and decoding bit vectors.
   *
   * This struct provides an encoding mechanism compatible with Rust's
   * `BitVec<u8, Lsb0>`. It supports efficient serialization and deserialization
   * using SCALE encoding.
   *
   * Internal Storage:
   * - Uses a single integral type T to store both size and data bits.
   * - The upper bits store the size, while the lower bits store the data.
   * - Efficient memory usage by storing size in the most significant bits.
   *
   * Example Usage:
   * @code
   * scale::SmallBitVector<> vec;
   * vec.push_back(true);
   * vec.push_back(false);
   * vec.push_back(true);
   * for (auto bit : vec) {
   *     std::cout << bit;
   * }
   * // Output: 101
   * @endcode
   */
  template <typename T = uintmax_t>
    requires std::is_integral_v<T> and std::is_unsigned_v<T>
  class SmallBitVector {
#ifdef CFG_TESTING
   public:
#endif
    //==========================================================================
    // Static Constants
    //==========================================================================

    /// @brief Number of bits in the type T.
    static constexpr size_t all_bits = std::numeric_limits<T>::digits;

    /**
     * @brief Number of bits needed to store the size.
     * @details Calculated based on the total number of bits in T.
     */
    static constexpr size_t size_bits = (all_bits >= 522)   ? 10
                                        : (all_bits >= 265) ? 9
                                        : (all_bits >= 136) ? 8
                                        : (all_bits >= 71)  ? 7
                                        : (all_bits >= 38)  ? 6
                                        : (all_bits >= 21)  ? 5
                                        : (all_bits >= 12)  ? 4
                                                            : 3;

    /// @brief Number of bits available for data.
    static constexpr size_t data_bits = all_bits - size_bits;

    /// @brief Mask for extracting size bits.
    static constexpr T size_mask = static_cast<T>(-1) << data_bits;

    /// @brief Mask for extracting data bits.
    static constexpr T data_mask = static_cast<T>(-1) >> size_bits;

   public:
    //==========================================================================
    // Nested Types
    //==========================================================================

    /**
     * @brief Proxy class for accessing and modifying individual bits.
     *
     * This class provides a proxy for modifying bits in the SmallBitVector.
     */
    class BitReference {
     public:
      /**
       * @brief Constructs a BitReference.
       * @param vec Reference to the SmallBitVector.
       * @param index Index of the bit.
       */
      BitReference(SmallBitVector &vec, size_t index) noexcept
          : vec_(vec), index_(index) {}

      /**
       * @brief Converts the bit to a boolean value.
       * @return True if the bit is set, false otherwise.
       */
      operator bool() const noexcept {
        return static_cast<const SmallBitVector &>(vec_)[index_];
      }

      /**
       * @brief Assigns a boolean value to the bit.
       * @param value The value to assign.
       * @return Reference to this BitReference.
       */
      BitReference &operator=(bool value) noexcept {
        if (value) {
          vec_.bits_ |= (static_cast<T>(1) << index_);
        } else {
          vec_.bits_ &= ~(static_cast<T>(1) << index_);
        }
        return *this;
      }

     private:
      SmallBitVector &vec_;
      size_t index_;
    };

    /**
     * @brief Iterator for traversing bits in SmallBitVector.
     *
     * This iterator provides random access to bits stored in the
     * SmallBitVector. It supports standard iterator operations such as
     * increment, dereference, and comparison.
     */
    class Iterator {
     public:
      /// @brief Iterator category indicating random access capability.
      using iterator_category = std::random_access_iterator_tag;
      /// @brief Type of values pointed to by the iterator.
      using value_type = bool;
      /// @brief Type used for distance between iterators.
      using difference_type = std::ptrdiff_t;

      /**
       * @brief Constructs an iterator for SmallBitVector.
       * @param vec Reference to the SmallBitVector.
       * @param pos Position of the current bit in the vector.
       */
      Iterator(const SmallBitVector &vec, size_t pos) noexcept
          : vec_(&vec), pos_(pos) {}

      /**
       * @brief Dereferences the iterator to access the current bit.
       * @return True if the current bit is set, false otherwise.
       */
      bool operator*() const noexcept {
        return (*vec_)[pos_];
      }

      /**
       * @brief Prefix increment. Moves the iterator to the next bit.
       * @return Reference to the updated iterator.
       */
      Iterator &operator++() noexcept {
        ++pos_;
        return *this;
      }

      /**
       * @brief Postfix increment. Moves the iterator to the next bit.
       * @return Copy of the iterator before increment.
       */
      Iterator operator++(int) noexcept {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      /**
       * @brief Difference operator for calculating distance between iterators.
       * @param other Iterator to calculate difference with.
       * @return Difference in positions.
       */
      difference_type operator-(const Iterator &other) const noexcept {
        return static_cast<difference_type>(pos_)
               - static_cast<difference_type>(other.pos_);
      }

      /**
       * @brief Equality comparison operator.
       * @param other Iterator to compare with.
       * @return True if both iterators are at the same position.
       */
      bool operator==(const Iterator &other) const noexcept {
        return pos_ == other.pos_;
      }

      /**
       * @brief Inequality comparison operator.
       * @param other Iterator to compare with.
       * @return True if iterators are at different positions.
       */
      bool operator!=(const Iterator &other) const noexcept {
        return !(*this == other);
      }

      /**
       * @brief Returns the current position of the iterator.
       * @return Current bit position.
       */
      size_t pos() const noexcept {
        return pos_;
      }

     private:
      const SmallBitVector *vec_;
      size_t pos_;
    };

    //==========================================================================
    // Constructors and Assignment Operators
    //==========================================================================

    /// @brief Default constructor. Initializes the vector as empty.
    SmallBitVector() noexcept : bits_(0) {}

    /**
     * @brief Constructs a SmallBitVector from an unsigned integral value.
     * @tparam I Integral type that fits within T.
     * @param bits Initial bits for the vector.
     */
    template <typename I>
      requires std::is_integral_v<I> and std::is_unsigned_v<I>
               and (sizeof(I) <= sizeof(T))
    explicit SmallBitVector(I bits) noexcept : bits_(bits) {}

    /// @brief Default copy constructor.
    SmallBitVector(const SmallBitVector &) noexcept = default;

    /// @brief Default move constructor.
    SmallBitVector(SmallBitVector &&) noexcept = default;

    /// @brief Default copy assignment operator.
    SmallBitVector &operator=(const SmallBitVector &) noexcept = default;

    /// @brief Default move assignment operator.
    SmallBitVector &operator=(SmallBitVector &&) noexcept = default;

    /**
     * @brief Constructs a SmallBitVector from a collection of boolean values.
     * @tparam InputIt Type of the input iterator.
     * @param first Iterator pointing to the first element in the collection.
     * @param last Iterator pointing to one past the last element in the
     * collection.
     * @throws std::overflow_error If the collection size exceeds the capacity.
     */
    template <typename InputIt>
    SmallBitVector(InputIt first, InputIt last) {
      size_t new_size = std::distance(first, last);
      if (new_size > data_bits) {
        throw std::overflow_error("Collection size exceeds capacity");
      }

      T new_data = 0;
      size_t index = 0;
      for (auto it = first; it != last; ++it) {
        if (*it) {
          new_data |= (static_cast<T>(1) << index);
        }
        ++index;
      }

      bits_ = (new_size << data_bits) | (new_data & data_mask);
    }

    /**
     * @brief Constructs a SmallBitVector from an initializer list of boolean
     * values.
     * @param init_list Initializer list of boolean values.
     * @throws std::overflow_error If the list size exceeds the capacity.
     */
    SmallBitVector(std::initializer_list<bool> init_list)
        : SmallBitVector(init_list.begin(), init_list.end()) {}

    /**
     * @brief Constructs a SmallBitVector from a collection of boolean values.
     * @tparam Collection Type of the collection (e.g., std::vector<bool>,
     * std::list<bool>).
     * @param collection The collection containing boolean values.
     * @throws std::overflow_error If the collection size exceeds the capacity.
     */
    template <typename Collection>
      requires std::is_class_v<Collection>
    explicit SmallBitVector(const Collection &collection)
        : SmallBitVector(collection.begin(), collection.end()) {}

    /**
     * @brief Assigns the contents of a collection to the SmallBitVector.
     * @tparam Collection Type of the collection (e.g., std::vector<bool>,
     * std::list<bool>).
     * @param collection The collection containing boolean values.
     * @return Reference to this SmallBitVector.
     * @throws std::overflow_error If the collection size exceeds the capacity.
     */
    template <typename Collection>
    SmallBitVector &operator=(const Collection &collection) {
      size_t new_size = collection.size();
      if (new_size > data_bits) {
        throw std::overflow_error("Collection size exceeds capacity");
      }

      T new_data = 0;
      size_t index = 0;
      for (const auto &value : collection) {
        if (value) {
          new_data |= (static_cast<T>(1) << index);
        }
        ++index;
      }

      bits_ = (new_size << data_bits) | (new_data & data_mask);
      return *this;
    }

    //==========================================================================
    // Capacity and Element Access
    //==========================================================================

    /**
     * @brief Converts the SmallBitVector to the underlying type T.
     * @return The bits stored in the vector.
     */
    explicit operator T() const noexcept {
      return bits_;
    }

    /**
     * @brief Returns the number of elements (bits) in the vector.
     * @return The current size of the vector.
     */
    [[nodiscard]] size_t size() const noexcept {
      return bits_ >> data_bits;
    }

    /**
     * @brief Returns the maximum capacity of the vector.
     * @return Maximum number of bits that can be stored.
     */
    [[nodiscard]] size_t capacity() const noexcept {
      return data_bits;
    }

    /**
     * @brief Checks if the vector is empty.
     * @return True if the vector is empty, false otherwise.
     */
    [[nodiscard]] bool empty() const noexcept {
      return size() == 0;
    }

    /**
     * @brief Provides read-only access to the bit at the given index.
     * @param index Position of the bit to access.
     * @return True if the bit is set, false otherwise.
     */
    bool operator[](size_t index) const noexcept {
      return (bits_ >> index) & static_cast<T>(1);
    }

    /**
     * @brief Provides mutable access to the bit at the given index.
     * @param index Position of the bit to access.
     * @return A BitReference proxy for modifying the bit.
     */
    BitReference operator[](size_t index) noexcept {
      return BitReference(*this, index);
    }

    /**
     * @brief Accesses the bit at the given index with bounds checking.
     * @param index Position of the bit to access.
     * @return True if the bit is set, false otherwise.
     * @throws std::out_of_range If the index is out of bounds.
     */
    bool at(size_t index) const {
      if (index >= size()) {
        throw std::out_of_range("Index out of bound");
      }
      return (*this)[index];
    }

    /**
     * @brief Returns the raw data bits of the vector.
     * @return The data bits stored in the vector.
     */
    [[nodiscard]] T data() const noexcept {
      return bits_ & data_mask;
    }

    //==========================================================================
    // Modifiers
    //==========================================================================

    /**
     * @brief Clears the vector, setting all bits to zero.
     */
    void clear() noexcept {
      bits_ = 0;
    }

    /**
     * @brief Reserves capacity for at least the specified number of bits.
     * @param new_capacity Minimum number of bits to reserve.
     * @throws std::overflow_error If the requested capacity exceeds the maximum
     * capacity.
     */
    void reserve(size_t new_capacity) {
      if (new_capacity > data_bits) {
        throw std::overflow_error(
            "Requested capacity exceeds maximum capacity");
      }
      // No actual allocation needed since the capacity is fixed by data_bits.
    }

    /**
     * @brief Resizes the vector to the new size.
     * @param new_size New size of the vector.
     * @throws std::out_of_range If the new size exceeds capacity.
     */
    void resize(size_t new_size) {
      if (new_size > data_bits) {
        throw std::out_of_range("New size exceeds capacity");
      }
      // Preserve current data bits up to new_size.
      T current_data = bits_ & data_mask;
      T new_data = current_data & ((static_cast<T>(1) << new_size) - 1);
      bits_ = (new_size << data_bits) | new_data;
    }

    /**
     * @brief Resizes the vector to the new size and fills new bits with a given
     * value.
     * @param new_size New size of the vector.
     * @param value Value to fill new bits (true or false).
     * @throws std::out_of_range If the new size exceeds capacity.
     */
    void resize(size_t new_size, bool value) {
      size_t current_size = size();
      if (new_size == current_size) return;
      if (new_size > data_bits) {
        throw std::out_of_range("New size exceeds capacity");
      }

      T current_data = bits_ & data_mask;

      if (new_size > current_size) {
        // Expanding the vector
        size_t added_bits = new_size - current_size;
        T new_bits = value ? ((static_cast<T>(1) << added_bits) - 1) : 0;
        new_bits <<= current_size;
        current_data |= new_bits;
      } else {
        // Shrinking the vector
        current_data &= ((static_cast<T>(1) << new_size) - 1);
      }

      bits_ = (new_size << data_bits) | current_data;
    }

    /**
     * @brief Adds a new bit to the end of the vector.
     * @param value Value of the new bit (true or false).
     * @throws std::overflow_error If the vector exceeds its capacity.
     */
    void push_back(bool value) {
      size_t current_size = size();
      if (current_size >= data_bits) {
        throw std::overflow_error("Exceeded maximum capacity");
      }
      // Update size.
      bits_ = ((current_size + 1) << data_bits) | (bits_ & data_mask);
      if (value) {
        bits_ |= static_cast<T>(1) << current_size;
      }
    }

    /**
     * @brief Removes the last bit from the vector.
     * @throws std::out_of_range If the vector is empty.
     */
    void pop_back() {
      size_t current_size = size();
      if (current_size == 0) {
        throw std::out_of_range("pop_back on empty vector");
      }
      // Clear the last bit.
      bits_ &= ~(static_cast<T>(1) << (current_size - 1));
      // Update size.
      bits_ = ((current_size - 1) << data_bits) | (bits_ & data_mask);
    }

    /**
     * @brief Inserts a bit at the specified position.
     * @param pos Position at which to insert the new bit.
     * @param value Value of the new bit.
     * @throws std::overflow_error If the vector exceeds its capacity.
     * @throws std::out_of_range If the position is out of bounds.
     */
    void insert(size_t pos, bool value) {
      size_t current_size = size();
      if (current_size >= data_bits) {
        throw std::overflow_error("Exceeded maximum capacity");
      }
      if (pos > current_size) {
        throw std::out_of_range("Insert position out of range");
      }
      T data_val = bits_ & data_mask;
      // Shift bits at and above pos to the left by one.
      T lower = data_val & ((static_cast<T>(1) << pos) - 1);
      T upper = data_val & ~((static_cast<T>(1) << pos) - 1);
      upper <<= 1;
      T new_data = lower | upper;
      // Set the inserted bit.
      if (value) {
        new_data |= static_cast<T>(1) << pos;
      } else {
        new_data &= ~(static_cast<T>(1) << pos);
      }
      bits_ = ((current_size + 1) << data_bits) | (new_data & data_mask);
    }

    /**
     * @brief Inserts multiple bits with the same value at the specified
     * position.
     * @param pos Iterator pointing to the position at which to insert the new
     * bits.
     * @param count Number of bits to insert.
     * @param value The value to assign to each inserted bit (true or false).
     * @throws std::overflow_error If the vector exceeds its capacity.
     * @throws std::out_of_range If the position is out of bounds.
     */
    void insert(Iterator pos, size_t count, bool value) {
      size_t current_size = size();
      if (count == 0) return;
      if (pos.pos() > current_size) {
        throw std::out_of_range("Insert position out of range");
      }
      if (current_size + count > data_bits) {
        throw std::overflow_error("Exceeded maximum capacity");
      }

      T data_val = bits_ & data_mask;
      T lower = data_val & ((static_cast<T>(1) << pos.pos()) - 1);
      T upper = data_val & ~((static_cast<T>(1) << pos.pos()) - 1);
      upper <<= count;

      T new_data = lower | upper;
      size_t insert_pos = pos.pos();
      for (size_t i = 0; i < count; ++i) {
        if (value) {
          new_data |= (static_cast<T>(1) << insert_pos);
        } else {
          new_data &= ~(static_cast<T>(1) << insert_pos);
        }
        ++insert_pos;
      }
      bits_ = ((current_size + count) << data_bits) | (new_data & data_mask);
    }

    /**
     * @brief Inserts a range of bits into the vector at the specified position.
     * @tparam InputIt Type of the input iterator.
     * @param pos Iterator pointing to the position at which to insert the new
     * bits.
     * @param first Iterator pointing to the first element in the range.
     * @param last Iterator pointing to one past the last element in the range.
     * @throws std::overflow_error If the vector exceeds its capacity.
     * @throws std::out_of_range If the position is out of bounds.
     */
    template <typename InputIt>
    void insert(Iterator pos, InputIt first, InputIt last) {
      size_t current_size = size();
      size_t new_bits_count = std::distance(first, last);
      if (new_bits_count == 0) return;
      if (pos.pos() > current_size) {
        throw std::out_of_range("Insert position out of range");
      }
      if (current_size + new_bits_count > data_bits) {
        throw std::overflow_error("Exceeded maximum capacity");
      }

      T data_val = bits_ & data_mask;
      T lower = data_val & ((static_cast<T>(1) << pos.pos()) - 1);
      T upper = data_val & ~((static_cast<T>(1) << pos.pos()) - 1);
      upper <<= new_bits_count;

      T new_data = lower | upper;
      size_t insert_pos = pos.pos();
      for (auto it = first; it != last; ++it) {
        if (*it) {
          new_data |= (static_cast<T>(1) << insert_pos);
        } else {
          new_data &= ~(static_cast<T>(1) << insert_pos);
        }
        ++insert_pos;
      }
      bits_ = ((current_size + new_bits_count) << data_bits)
              | (new_data & data_mask);
    }

    /**
     * @brief Erases the bit at the specified position.
     * @param pos Position of the bit to erase.
     * @throws std::out_of_range If the position is out of bounds.
     */
    void erase(size_t pos) {
      size_t current_size = size();
      if (pos >= current_size) {
        throw std::out_of_range("Erase position out of range");
      }
      T data_val = bits_ & data_mask;
      T lower = data_val & ((static_cast<T>(1) << pos) - 1);
      T upper = data_val >> (pos + 1);
      T new_data = lower | (upper << pos);
      bits_ = ((current_size - 1) << data_bits) | (new_data & data_mask);
    }

    /**
     * @brief Assigns the vector with the specified number of bits, all set to a
     * given value.
     * @param count Number of bits to assign.
     * @param value The value to assign to each bit.
     * @throws std::out_of_range If count exceeds capacity.
     */
    void assign(size_t count, bool value) {
      if (count > data_bits) {
        throw std::out_of_range("Assign count exceeds capacity");
      }
      T new_data = value ? ((static_cast<T>(1) << count) - 1) : 0;
      bits_ = (count << data_bits) | (new_data & data_mask);
    }

    /**
     * @brief Swaps the contents of this vector with another.
     * @param other The other SmallBitVector to swap with.
     */
    void swap(SmallBitVector &other) noexcept {
      std::swap(bits_, other.bits_);
    }

    //==========================================================================
    // Iterators
    //==========================================================================

    /**
     * @brief Returns an iterator to the beginning of the vector.
     * @return Iterator pointing to the first bit.
     */
    Iterator begin() const noexcept {
      return Iterator(*this, 0);
    }

    /**
     * @brief Returns an iterator to the end of the vector.
     * @return Iterator pointing to one past the last bit.
     */
    Iterator end() const noexcept {
      return Iterator(*this, size());
    }

    /**
     * @brief Checks if two SmallBitVector objects are equal.
     * @param other The SmallBitVector to compare with.
     * @return true if both SmallBitVectors are equal, false otherwise
     */
    bool operator==(const SmallBitVector &other) const noexcept {
      return this->size() == other.size() && this->data() == other.data();
    }

    /**
     * @brief Checks if two SmallBitVector objects are not equal.
     * @param other The SmallBitVector to compare with.
     * @return true if SmallBitVectors are not equal, false otherwise
     */
    bool operator!=(const SmallBitVector &other) const noexcept {
      return !(*this == other);
    }

    //==========================================================================
    // Friend Functions (Encoding/Decoding)
    //==========================================================================

    /**
     * @brief Encodes the bit vector into a SCALE encoder using bit
     * manipulation.
     * @param bit_vector The bit vector to encode.
     * @param encoder The encoder instance to write into.
     */
    friend void encode(const SmallBitVector &bit_vector, Encoder &encoder) {
      size_t size = bit_vector.size();
      encode(as_compact(size), encoder);
      T data_val = bit_vector.bits_ & data_mask;
      for (size_t i = 0; i < size; i += CHAR_BIT) {
        encoder.put(static_cast<uint8_t>(data_val >> i));
      }
    }

    /**
     * @brief Decodes a BitVec from a SCALE decoder.
     * @param v The BitVec instance to populate.
     * @param decoder The decoder instance to read from.
     * @throws DecodeError::NOT_ENOUGH_DATA If there is insufficient data.
     */
    friend void decode(SmallBitVector &v, Decoder &decoder) {
      size_t size;
      decode(as_compact(size), decoder);
      size_t byte_count = (size + CHAR_BIT - 1) / CHAR_BIT;
      if (!decoder.has(byte_count)) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      T bits = 0;
      for (size_t i = 0; i < size; i += CHAR_BIT) {
        uintmax_t byte = decoder.take();
        bits |= static_cast<T>(byte) << i;
      }
      v = SmallBitVector(bits);
    }

   private:
    //==========================================================================
    // Private Data Members
    //==========================================================================

    /// Internal storage combining size and data bits.
    T bits_;
  };

  /**
   * @class BitVector
   * @brief A dynamic bit vector with Small Buffer Optimization (SBO).
   *
   * This class implements a bit vector with the following features:
   * - Small Buffer Optimization: Stores small bit sequences in a fixed-size
   * array.
   * - Dynamic resizing with automatic transition to dynamic vector storage.
   * - Proxy class BitReference for safe bit manipulation.
   * - Random-access iterators for compatibility with STL algorithms.
   */
  class BitVector {
   public:
    //==========================================================================
    // Nested Classes
    //==========================================================================

    /**
     * @class BitReference
     * @brief Proxy class for accessing and modifying individual bits.
     *
     * This class allows safe access and modification of bits within
     BitVector.
     * It supports conversion to bool and assignment of boolean values.
     */
    class BitReference {
     public:
      /**
       * @brief Constructs a BitReference.
       * @param vec Reference to the parent BitVector.
       * @param index Index of the bit within the BitVector.
       */
      BitReference(BitVector &vec, size_t index) noexcept
          : vec_(vec), index_(index) {}

      /**
       * @brief Converts the bit to a boolean value.
       * @return true if the bit is set, false otherwise.
       */
      operator bool() const noexcept {
        return static_cast<const BitVector &>(vec_)[index_];
      }

      /**
       * @brief Assigns a boolean value to the bit.
       * @param value The value to assign to the bit.
       * @return Reference to this BitReference.
       */
      BitReference &operator=(bool value) noexcept {
        auto data = vec_.sbf_ ? vec_.arr_.data() : vec_.vec_.data();
        auto &byte = data[index_ / CHAR_BIT];
        auto bit = index_ % CHAR_BIT;
        if (value) {
          byte |= (static_cast<uint8_t>(1) << bit);  // Set the bit
        } else {
          byte &= ~(static_cast<uint8_t>(1) << bit);  // Clear the bit
        }
        return *this;
      }

      /**
       * @brief Assigns the value of another BitReference.
       * @param other The BitReference to copy from.
       * @return Reference to this BitReference.
       */
      BitReference &operator=(const BitReference &other) noexcept {
        return *this = static_cast<bool>(other);
      }

      /**
       * @brief Equality comparison operator.
       * @param other The BitReference to compare with.
       * @return true if the bits are equal, false otherwise.
       */
      bool operator==(const BitReference &other) const noexcept {
        return static_cast<bool>(*this) == static_cast<bool>(other);
      }

      /**
       * @brief Inequality comparison operator.
       * @param other The BitReference to compare with.
       * @return true if the bits are not equal, false otherwise.
       */
      bool operator!=(const BitReference &other) const noexcept {
        return !(*this == other);
      }

     private:
      BitVector &vec_;  ///< Reference to the parent BitVector
      size_t index_;    ///< Index of the bit within the BitVector
    };

    /**
     * @class Iterator
     * @brief Iterator for BitVector, providing random access to its
     elements.
     * This iterator allows traversal over a BitVector in a random-access
     * manner. It provides the standard iterator interface, including
     * dereference, increment (both prefix and postfix), and comparison
     * operators.
     *
     * This version supports std::distance by implementing subtraction and
     * comparison operators, enabling the use of random access iterator
     * features. Designed with C++20 concepts and modern practices.
     *
     * This iterator works with BitReference proxy class, since references to
     * individual bits cannot be used.
     */
    class Iterator {
     public:
      /// Concept of the iterator for C++20 ranges.
      using iterator_concept = std::random_access_iterator_tag;
      /// Category of the iterator, indicating random access capabilities.
      using iterator_category = std::random_access_iterator_tag;
      /// Type of the elements pointed to by the iterator (bool for BitVector).
      using value_type = bool;
      /// Type used for distance between iterators.
      using difference_type = std::ptrdiff_t;
      /// Pointer type (not applicable but required for iterator traits).
      using pointer = const bool *;
      /// Reference type (using proxy class for a bit reference).
      using reference = BitReference;

      /**
       * @brief Constructs an iterator for the given BitVector at a specified
       * position.
       * @param vec Reference to the BitVector to iterate over.
       * @param pos Initial position of the iterator within the BitVector.
       */
      constexpr Iterator(BitVector &vec, size_t pos) noexcept
          : vec_(std::addressof(vec)), pos_(pos) {}

      /**
       * @brief Constructs an iterator for the given const BitVector at a
       * specified position.
       * @param vec Reference to the const BitVector to iterate over.
       * @param pos Initial position of the iterator within the BitVector.
       */
      constexpr Iterator(const BitVector &vec, size_t pos) noexcept
          : vec_(std::addressof(vec)), pos_(pos) {}

      /**
       * @brief Dereferences the iterator to access the current element.
       * @return The BitReference at the current iterator position.
       * @note This uses a proxy class since references to bits are not
       * possible.
       */
      [[nodiscard]] BitReference operator*() noexcept {
        return BitReference{*const_cast<BitVector *>(vec_), pos_};
      }

      /**
       * @brief Dereferences the iterator to access the current element
       (const
       * version).
       * @return The value of the bit at the current iterator position as
       bool.
       * @note Const version returns a bool since no modification is allowed.
       */
      [[nodiscard]] bool operator*() const noexcept {
        return (*vec_)[pos_];
      }

      /**
       * @brief Prefix increment operator.
       * Moves the iterator to the next position.
       * @return Reference to the incremented iterator.
       */
      Iterator &operator++() noexcept {
        ++pos_;
        return *this;
      }

      /**
       * @brief Postfix increment operator.
       * Moves the iterator to the next position, returning the previous
       state.
       * @return Copy of the iterator before incrementing.
       */
      Iterator operator++(int) noexcept {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      /**
       * @brief Equality comparison operator.
       * Checks if two iterators are equal.
       * @param other Iterator to compare with.
       * @return True if iterators are equal, false otherwise.
       */
      bool operator==(const Iterator &other) const noexcept {
        return pos_ == other.pos_;
      }

      /**
       * @brief Inequality comparison operator.
       * Checks if two iterators are not equal.
       * @param other Iterator to compare with.
       * @return True if iterators are not equal, false otherwise.
       */
      bool operator!=(const Iterator &other) const noexcept {
        return !(*this == other);
      }

      /**
       * @brief Subtraction operator for distance calculation.
       * Allows calculation of the distance between two iterators.
       * @param lhs Left-hand side iterator.
       * @param rhs Right-hand side iterator.
       * @return Distance between the iterators as a difference_type.
       */
      friend difference_type operator-(const Iterator &lhs,
                                       const Iterator &rhs) noexcept {
        return static_cast<difference_type>(lhs.pos_)
               - static_cast<difference_type>(rhs.pos_);
      }

     private:
      /// Pointer to the BitVector being iterated over.
      const BitVector *vec_;
      /// Current position of the iterator within the BitVector.
      size_t pos_;
    };

    //==========================================================================
    // Constructors, Destructor, and Assignment Operators
    //==========================================================================

    /**
     * @brief Constructs an empty BitVector.
     * Initializes the BitVector with a size of 0 and enables small buffer
     * optimization (SBO) by default.
     */
    BitVector() : size_(0), sbf_(1), arr_{0} {}

    /**
     * @brief Constructs a BitVector from an initializer list.
     * @param init_list The initializer list with boolean values.
     */
    BitVector(std::initializer_list<bool> init_list) {
      resize(init_list.size());
      size_t index = 0;
      for (bool value : init_list) {
        (*this)[index++] = value;
      }
    }

    /**
     * @brief Constructs a BitVector from any container of boolean values.
     * This constructor allows initializing the BitVector using any container
     * that supports `std::begin()` and `std::end()`, including
     * std::vector<bool>.
     * @tparam Container A container type that holds boolean values.
     * @param container The container with boolean values to initialize the
     * BitVector.
     */
    template <typename Container>
    explicit BitVector(const Container &container) {
      size_t count = std::distance(std::begin(container), std::end(container));
      resize(count);
      size_t index = 0;
      for (const auto &value : container) {
        (*this)[index++] = value;
      }
    }

    /**
     * @brief Copy constructor for BitVector.
     * Creates a new BitVector as a copy of the given one.
     * @param other The BitVector to copy from.
     */
    BitVector(const BitVector &other) : size_(other.size_), sbf_(other.sbf_) {
      if (sbf_) {
        std::memcpy(arr_.data(), other.arr_.data(), other.arr_.size());
      } else if (other.size() <= arr_.size() * CHAR_BIT) {
        sbf_ = 1;
        std::memcpy(arr_.data(), other.vec_.data(), other.vec_.size());
      } else {
        new (&vec_) std::vector<uint8_t>(other.vec_);
      }
    }

    /**
     * @brief Move constructor for BitVector.
     * Moves the contents from another BitVector.
     * @param other The BitVector to move from.
     */
    BitVector(BitVector &&other) noexcept
        : size_(other.size_), sbf_(other.sbf_) {
      if (sbf_) {
        std::memcpy(arr_.data(), other.arr_.data(), other.arr_.size());
      } else {
        new (&vec_) std::vector<uint8_t>(std::move(other.vec_));
      }
      other.size_ = 0;
      other.sbf_ = 1;
      other.arr_[0] = 0;
    }

    /**
     * @brief Copy assignment operator for BitVector.
     * Copies the contents from another BitVector.
     * @param other The BitVector to copy from.
     * @return Reference to this BitVector.
     */
    BitVector &operator=(const BitVector &other) {
      if (this != &other) {
        this->~BitVector();           // Destroy current state
        new (this) BitVector(other);  // Recreate using copy constructor
      }
      return *this;
    }

    /**
     * @brief Move assignment operator for BitVector.
     * Moves the contents from another BitVector.
     * @param other The BitVector to move from.
     * @return Reference to this BitVector.
     */
    BitVector &operator=(BitVector &&other) noexcept {
      if (this != &other) {
        this->~BitVector();  // Destroy current state
        new (this)
            BitVector(std::move(other));  // Recreate using move constructor
      }
      return *this;
    }

    /**
     * @brief Assigns the contents of the BitVector from an initializer list.
     * @param init_list The initializer list with boolean values.
     * @return Reference to this BitVector.
     */
    BitVector &operator=(std::initializer_list<bool> init_list) {
      resize(init_list.size());
      size_t index = 0;
      for (bool value : init_list) {
        (*this)[index++] = value;
      }
      return *this;
    }

    /**
     * @brief Assigns the contents of the BitVector from any container of
     * boolean values. This operator allows assignment from any container
     that
     * supports `std::begin()` and `std::end()`, including std::vector<bool>.
     * @tparam Container A container type that holds boolean values.
     * @param container The container with boolean values to assign to the
     * BitVector.
     * @return Reference to this BitVector.
     */
    template <typename Container>
    BitVector &operator=(const Container &container) {
      size_t count = std::distance(std::begin(container), std::end(container));
      resize(count);
      size_t index = 0;
      for (const auto &value : container) {
        (*this)[index++] = value;
      }
      return *this;
    }

    /**
     * @brief Destructor for BitVector.
     * If small buffer optimization is disabled, the internal vector is
     * explicitly destroyed to free the allocated memory.
     */
    ~BitVector() {
      if (sbf_ == 0) {
        std::destroy_at(&vec_);
      }
    }

    //==========================================================================
    // Capacity and Element Access
    //==========================================================================

    /**
     * @brief Returns the number of bits currently stored in the BitVector.
     * @return The size of the BitVector in bits.
     */
    [[nodiscard]] size_t size() const noexcept {
      return size_;
    }

    /**
     * @brief Returns the capacity of the BitVector in bits.
     * Capacity is the total number of bits that the BitVector can hold
     * without requiring reallocation.
     * @return The capacity of the BitVector in bits.
     */
    [[nodiscard]] size_t capacity() const noexcept {
      if (sbf_) {
        return sizeof(arr_) * CHAR_BIT;
      }
      return vec_.size() * CHAR_BIT;
    }

    /**
     * @brief Checks if the BitVector is empty.
     * @return True if the BitVector has no bits, otherwise false.
     */
    [[nodiscard]] bool empty() const noexcept {
      return size_ == 0;
    }

    /**
     * @brief Provides non-const access to the bit at the specified index.
     * @param index The position of the bit to access.
     * @return A BitReference allowing modification of the specified bit.
     * @note It is the caller's responsibility to ensure the index is within
     * bounds.
     */
    BitReference operator[](size_t index) noexcept {
      return {*this, index};
    }

    /**
     * @brief Provides read-only access to the bit at the specified index.
     * @param index The position of the bit to read.
     * @return The value of the bit (true for 1, false for 0).
     * @note It is the caller's responsibility to ensure the index is within
     * bounds.
     */
    bool operator[](size_t index) const noexcept {
      auto data = sbf_ ? arr_.data() : vec_.data();
      auto &byte = data[index / CHAR_BIT];
      auto bit = index % CHAR_BIT;
      return byte & (static_cast<uint8_t>(1) << bit);
    }

    /**
     * @brief Accesses the bit at the specified index with bounds checking.
     * @param index The position of the bit to access.
     * @return A BitReference allowing modification of the specified bit.
     * @throws std::out_of_range If the index is out of bounds.
     */
    BitReference at(size_t index) {
      if (index < size_) {
        return (*this)[index];
      }
      throw std::out_of_range("BitVector::at - Index out of range");
    }

    /**
     * @brief Provides read-only access to the bit at the specified index
     with
     * bounds checking.
     * @param index The position of the bit to read.
     * @return The value of the bit (true for 1, false for 0).
     * @throws std::out_of_range If the index is out of bounds.
     */
    [[nodiscard]] bool at(size_t index) const {
      if (index < size_) {
        return (*this)[index];
      }
      throw std::out_of_range("BitVector::at - Index out of range");
    }

    /**
     * @brief Provides non-const access to the last bit
     * @return A BitReference allowing modification of the last bit.
     * @note It is the caller's responsibility to ensure the container
     non-empty
     */
    BitReference back() noexcept {
      return {*this, size_ - 1};
    }

    /**
     * @brief Returns a read-only span of the underlying bytes.
     * @return A span of bytes representing the bit data.
     */
    [[nodiscard]] std::span<const uint8_t> bytes() const noexcept {
      if (size_) {
        return {sbf_ ? arr_.data() : vec_.data(),
                (size_ + CHAR_BIT - 1) / CHAR_BIT};
      }
      return {};
    }

    //==========================================================================
    // Modifiers
    //==========================================================================

    /**
     * @brief Clears the BitVector, setting its size to 0.
     */
    void clear() noexcept {
      resize(0);
    }

    /**
     * @brief Reserves capacity for the BitVector.
     * Ensures that the BitVector can hold at least the specified number of
     bits
     * without reallocating. If the requested capacity is less than or equal
     to
     * the current capacity, no action is taken.
     * @param new_capacity The minimum capacity in bits to reserve.
     */
    void reserve(size_t new_capacity) {
      size_t required_bytes = (new_capacity + CHAR_BIT - 1) / CHAR_BIT;

      // If current capacity is already sufficient, do nothing
      if (new_capacity <= capacity()) {
        return;
      }

      // Switch to dynamic vector if needed
      if (sbf_) {
        switch_to_vector();
      }

      // Reserve space in the dynamic vector
      vec_.reserve(required_bytes);
    }

    /**
     * @brief Resizes the BitVector to the specified number of bits.
     * @param new_size The new size in bits.
     * If the new size is larger, the new bits are initialized to 0.
     * If smaller, the BitVector is truncated.
     */
    void resize(size_t new_size) {
      if (sbf_ and new_size > arr_.size() * CHAR_BIT) {
        switch_to_vector();
      }
      if (new_size <= size_) {
        auto data = sbf_ ? arr_.data() : vec_.data();
        data[new_size / CHAR_BIT] &=
            static_cast<uint8_t>(-1) >> (CHAR_BIT - (new_size % CHAR_BIT));
      }
      if (not sbf_) {
        if (new_size != vec_.size() * CHAR_BIT) {
          vec_.resize((new_size + CHAR_BIT - 1) / CHAR_BIT, 0);
        }
      }
      if (sbf_ and new_size > size_) {
        for (auto i = (size_ + CHAR_BIT - 1) / CHAR_BIT;
             i <= new_size / CHAR_BIT;
             ++i) {
          arr_[i] = 0;
        }
      }
      size_ = new_size;
    }

    /**
     * @brief Resizes the BitVector to the specified number of bits.
     * If the new size is larger, the new bits are initialized to the
     specified
     * value. If smaller, the BitVector is truncated.
     * @param new_size The new size in bits.
     * @param value The value to initialize new bits (true for 1, false for
     0).
     */
    void resize(size_t new_size, bool value) {
      if (new_size > size_) {
        // Store current size to know the starting point for new bits
        size_t old_size = size_;
        // Resize to the new size
        resize(new_size);
        // Initialize the new bits with the specified value
        for (size_t i = old_size; i < new_size; ++i) {
          (*this)[i] = value;
        }
      } else if (new_size < size_) {
        // If the new size is smaller, just truncate the BitVector
        resize(new_size);
      }
    }

    /**
     * @brief Adds a new bit at the end of the BitVector.
     * @param value The value of the new bit (true for 1, false for 0).
     */
    void push_back(bool value) {
      auto index = size_;
      resize(index + 1);
      if (value) {
        auto data = sbf_ ? arr_.data() : vec_.data();
        auto &byte = data[index / CHAR_BIT];
        auto bit = index % CHAR_BIT;
        byte |= (static_cast<uint8_t>(1) << bit);
      }
    }

    /**
     * @brief Inserts a bit at the specified index.
     * @param index The position where the new bit is inserted.
     * @param value The value of the new bit (true for 1, false for 0).
     * @throws std::out_of_range If the index is out of bounds.
     */
    void insert(size_t index, bool value) {
      if (index > size_) {
        throw std::out_of_range("BitVector::insert - Index out of range");
      }
      resize(size_ + 1);
      for (size_t i = size_ - 1; i > index; --i) {
        (*this)[i] = (*this)[i - 1];
      }
      (*this)[index] = value;
    }

    /**
     * @brief Inserts multiple copies of a value at the specified position.
     * @param pos Iterator pointing to the position where new bits are
     inserted.
     * @param count Number of bits to insert.
     * @param value The value of the new bits (true for 1, false for 0).
     * @throws std::out_of_range If the position is out of bounds.
     */
    void insert(Iterator pos, size_t count, bool value) {
      size_t index = std::distance(begin(), pos);
      // Check if position is within the valid range
      if (index > size_) {
        throw std::out_of_range("BitVector::insert - Position out of range");
      }
      // If no bits are to be inserted, return
      if (count == 0) {
        return;
      }
      // Resize to accommodate the new bits
      resize(size_ + count);
      // Shift existing bits to the right
      for (size_t i = size_ - 1; i >= index + count; --i) {
        (*this)[i] = (*this)[i - count];
      }
      // Fill the new space with the specified value
      for (size_t i = 0; i < count; ++i) {
        (*this)[index + i] = value;
      }
    }

    /**
     * @brief Inserts a range of bits at the specified position.
     * @tparam InputIt Iterator type for the range to insert.
     * @param pos Iterator pointing to the position where new bits are
     inserted.
     * @param first Iterator to the first element of the range to insert.
     * @param last Iterator to the end of the range to insert.
     * @throws std::out_of_range If the position is out of bounds.
     */
    template <typename InputIt>
    void insert(Iterator pos, InputIt first, InputIt last) {
      size_t index = std::distance(begin(), pos);
      // Check if position is within the valid range
      if (index > size_) {
        throw std::out_of_range("BitVector::insert - Position out of range");
      }
      // Calculate the number of bits to insert
      size_t count = std::distance(first, last);
      if (count == 0) {
        return;
      }
      // Resize to accommodate the new bits
      resize(size_ + count);
      // Shift existing bits to the right
      for (size_t i = size_ - 1; i >= index + count; --i) {
        (*this)[i] = (*this)[i - count];
      }
      // Copy new bits into the position
      size_t insert_pos = index;
      for (auto it = first; it != last; ++it, ++insert_pos) {
        (*this)[insert_pos] = *it;
      }
    }

    /**
     * @brief Assigns the BitVector with the specified number of bits.
     * @param count The number of bits to assign.
     * @param value The value to assign to each bit (true for 1, false for
     0).
     */
    void assign(size_t count, bool value) {
      resize(count);
      for (size_t i = 0; i < count; ++i) {
        (*this)[i] = value;
      }
    }

    //==========================================================================
    // Iterators
    //==========================================================================

    /**
     * @brief Returns an iterator to the beginning of the BitVector.
     * @return A constant iterator pointing to the first bit.
     */
    Iterator begin() noexcept {
      return {*this, 0};
    }

    /**
     * @brief Returns an iterator to the end of the BitVector.
     * @return A constant iterator pointing to one past the last bit.
     */
    Iterator end() noexcept {
      return {*this, size_};
    }

    /**
     * @brief Returns an iterator to the beginning of the BitVector.
     * @return A constant iterator pointing to the first bit.
     */
    [[nodiscard]] Iterator begin() const noexcept {
      return {*this, 0};
    }

    /**
     * @brief Returns an iterator to the end of the BitVector.
     * @return A constant iterator pointing to one past the last bit.
     */
    [[nodiscard]] Iterator end() const noexcept {
      return {*this, size_};
    }

    /**
     * @brief Checks if two BitVector objects are equal.
     * @param other The BitVector to compare with.
     * @return true if both BitVector are equal, false otherwise
     */
    bool operator==(const BitVector &other) const noexcept {
      if (this->size() != other.size()) {
        return false;
      }
      auto lhs_bytes = this->bytes();
      auto rhs_bytes = other.bytes();
      return std::equal(lhs_bytes.begin(), lhs_bytes.end(), rhs_bytes.begin());
    }

    /**
     * @brief Checks if two BitVector objects are unequal.
     * @param other The BitVector to compare with.
     * @return true if BitVector are unequal, false otherwise
     */
    bool operator!=(const BitVector &other) const noexcept {
      return !(*this == other);
    }

    //==========================================================================
    // Friend Functions (Encoding/Decoding)
    //==========================================================================

    /**
     * @brief Encodes the bit vector into a SCALE encoder.
     * @param bit_vector The bit vector to encode.
     * @param encoder The encoder instance to write into.
     */
    friend void encode(const BitVector &bit_vector, Encoder &encoder) {
      encode(as_compact(bit_vector.size()), encoder);
      auto data =
          bit_vector.sbf_ ? bit_vector.arr_.data() : bit_vector.vec_.data();
      size_t byte_size = (bit_vector.size() + CHAR_BIT - 1) / CHAR_BIT;
      for (size_t i = 0; i < byte_size; ++i) {
        encoder.put(data[i]);
      }
    }

    /**
     * @brief Decodes a BitVec from a SCALE decoder.
     * @param bit_vector The BitVec instance to populate.
     * @param decoder The decoder instance to read from.
     * @throws DecodeError::NOT_ENOUGH_DATA If there is insufficient data.
     */
    friend void decode(BitVector &bit_vector, Decoder &decoder) {
      size_t bit_size;
      decode(as_compact(bit_size), decoder);
      size_t byte_size = (bit_size + CHAR_BIT - 1) / CHAR_BIT;
      if (not decoder.has(byte_size)) {
        raise(DecodeError::NOT_ENOUGH_DATA);
      }
      bit_vector.resize(bit_size);
      auto data =
          bit_vector.sbf_ ? bit_vector.arr_.data() : bit_vector.vec_.data();
      for (size_t i = 0; i < byte_size; ++i) {
        data[i] = decoder.take();
      }
      if (auto last_bits = bit_size % CHAR_BIT; last_bits > 0) {
        if (byte_size > 0
            && (data[byte_size - 1]
                & (static_cast<uint8_t>(-1) << last_bits))) {
          raise(DecodeError::UNUSED_BITS_ARE_SET);
        }
      }
    }

   private:
    //==========================================================================
    // Private Helper Functions and Data Members
    //==========================================================================

    /**
     * @brief Switches from small buffer optimization to dynamic vector.
     * Moves data from the small fixed-size array to a dynamically allocated
     * vector when the size exceeds the fixed capacity.
     */
    void switch_to_vector() {
      std::vector vec(arr_.begin(), arr_.end());
      new (&vec_) std::vector<uint8_t>();
      vec_.swap(vec);
      sbf_ = 0;
    }

    /// Number of bits.
    size_t size_ : std::numeric_limits<size_t>::digits - 1;
    /// Small buffer optimization flag.
    size_t sbf_ : 1;
    union {
      /// Dynamic storage for large bit vectors.
      std::vector<uint8_t> vec_;
      /// Small buffer optimization.
      std::array<uint8_t, sizeof(decltype(vec_)) / sizeof(uint8_t)> arr_;
    };
  };
}  // namespace scale
