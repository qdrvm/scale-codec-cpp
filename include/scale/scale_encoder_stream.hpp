/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <boost/variant.hpp>

#include <scale/bitvec.hpp>
#include <scale/detail/fixed_width_integer.hpp>
#ifdef JAM_COMPATIBILITY_ENABLED
#include <scale/detail/jam_compact_integer.hpp>
#else
#include <scale/detail/compact_integer.hpp>
#endif
#include <scale/configurable.hpp>
#include <scale/scale_error.hpp>
#include <scale/types.hpp>

namespace scale {

  /**
   * @class ScaleEncoderStream designed to scale-encode data to stream
   */
  class ScaleEncoderStream : public Configurable {
   public:
    // special tag to differentiate encoding streams from others
    static constexpr auto is_encoder_stream = true;

    ScaleEncoderStream();

    /**
     * Stream initialization
     * @param drop_data - when true will only count encoded data size while
     * omitting the data itself
     */
    explicit ScaleEncoderStream(bool drop_data);

#ifdef CUSTOM_CONFIG_ENABLED
    explicit ScaleEncoderStream(const MaybeConfig auto &...configs)
        : Configurable(configs...) {}

    explicit ScaleEncoderStream(bool drop_data,
                                const MaybeConfig auto &...configs)
        : Configurable(configs...), drop_data_(drop_data) {}
#else
    [[deprecated("Scale has compiled without custom config support")]]  //
    ScaleEncoderStream(const MaybeCofing auto &...configs) = delete;

    [[deprecated("Scale has compiled without custom config support")]]  //
    ScaleEncoderStream(bool drop_data,
                       const MaybeCofing auto &...configs) = delete;
#endif

    /**
     * @return vector of bytes containing encoded data
     */
    std::vector<uint8_t> to_vector() const;

    /**
     * Get amount of encoded data written to the stream
     * @return size in bytes
     */
    size_t size() const;

    /**
     * @brief scale-encodes range
     * @param collection range to encode
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const DynamicCollection auto &collection) {
      return encodeDynamicCollection(collection);
    }

    ScaleEncoderStream &operator<<(const StaticCollection auto &collection) {
      return encodeStaticCollection(collection);
    }

    /**
     * @brief scale-encodes BitVec
     */
    ScaleEncoderStream &operator<<(const BitVec &v);

    /**
     * @brief scale-encodes pair of values
     * @tparam F first value type
     * @tparam S second value type
     * @param p pair of values to encode
     * @return reference to stream
     */
    template <class F, class S>
    ScaleEncoderStream &operator<<(const std::pair<F, S> &p) {
      return *this << p.first << p.second;
    }

    /**
     * @brief scale-encodes tuple
     * @tparam T enumeration of types
     * @param v tuple
     * @return reference to stream
     */
    template <class... Ts>
    ScaleEncoderStream &operator<<(const std::tuple<Ts...> &v) {
      if constexpr (sizeof...(Ts) > 0) {
        encodeElementOfTuple<0>(v);
      }
      return *this;
    }

    /**
     * @brief scale-encodes variant value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <class... T>
    ScaleEncoderStream &operator<<(const boost::variant<T...> &v) {
      tryEncodeAsOneOfVariant<0>(v);
      return *this;
    }

    /**
     * @brief scale-encodes sharead_ptr value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <class T>
    ScaleEncoderStream &operator<<(const std::shared_ptr<T> &v) {
      if (v == nullptr) {
        raise(EncodeError::DEREF_NULLPOINTER);
      }
      return *this << *v;
    }

    /**
     * @brief scale-encodes unique_ptr value
     * @tparam T type list
     * @param v value to encode
     * @return reference to stream
     */
    template <class T>
    ScaleEncoderStream &operator<<(const std::unique_ptr<T> &v) {
      if (v == nullptr) {
        raise(EncodeError::DEREF_NULLPOINTER);
      }
      return *this << *v;
    }

    /**
     * @brief scale-encodes optional value
     * @tparam T value type
     * @param v value to encode
     * @return reference to stream
     */
    template <class T>
    ScaleEncoderStream &operator<<(const std::optional<T> &v) {
      // optional bool is a special case of optional values
      // it should be encoded using one byte instead of two
      // as described in specification
      if constexpr (std::is_same_v<T, bool>) {
        return encodeOptionalBool(v);
      }
      if (!v.has_value()) {
        return putByte(0u);
      }
      return putByte(1u) << *v;
    }

    /**
     * @brief scale-encodes nullopt  value
     * @param v - std::nullopt only
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const std::nullopt_t &) {
      return putByte(0u);
    }

    /**
     * @brief scale-encodes std::reference_wrapper of a type
     * @tparam T underlying type
     * @param v value to encode
     * @return reference to stream;
     */
    template <class T>
    ScaleEncoderStream &operator<<(const std::reference_wrapper<T> &v) {
      return *this << static_cast<const T &>(v);
    }

    /**
     * @brief scale-encodes a string view
     * @param sv string_view item
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(std::string_view sv) {
      return encodeDynamicCollection(sv);
    }

    /**
     * @brief scale-encodes a vector of bool
     * @param v vector of bool
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const std::vector<bool> &v) {
      *this << CompactInteger{v.size()};
      for (bool el : v) {
        *this << el;
      }
      return *this;
    }

    /**
     * @brief scale-encodes any integral type including bool
     * @tparam T integral type
     * @param v value of integral type
     * @return reference to stream
     */
    template <typename T,
              typename I = std::decay_t<T>,
              typename = std::enable_if_t<std::is_integral_v<I>>>
    ScaleEncoderStream &operator<<(T &&v) {
      // encode bool
      if constexpr (std::is_same_v<I, bool>) {
        uint8_t byte = (v ? 1u : 0u);
        return putByte(byte);
      }
      // put byte
      if constexpr (sizeof(T) == 1u) {
        // to avoid infinite recursion
        return putByte(static_cast<uint8_t>(v));
      }
      // encode any other integer
      detail::encodeInteger<I>(v, *this);
      return *this;
    }

    /**
     * @brief scale-encodes CompactInteger value as compact integer
     * @param v value to encode
     * @return reference to stream
     */
    ScaleEncoderStream &operator<<(const CompactInteger &v);

   protected:
    template <size_t I, class... Ts>
    void encodeElementOfTuple(const std::tuple<Ts...> &v) {
      *this << std::get<I>(v);
      if constexpr (sizeof...(Ts) > I + 1) {
        encodeElementOfTuple<I + 1>(v);
      }
    }

    template <uint8_t I, class... Ts>
    void tryEncodeAsOneOfVariant(const boost::variant<Ts...> &v) {
      using T = std::tuple_element_t<I, std::tuple<Ts...>>;
      if (v.type() == typeid(T)) {
        *this << I << boost::get<T>(v);
        return;
      }
      if constexpr (sizeof...(Ts) > I + 1) {
        tryEncodeAsOneOfVariant<I + 1>(v);
      }
    }

    /**
     * @brief scale-encodes any dynamic collection
     * @param collection encoding collection
     * @return reference to stream
     */
    ScaleEncoderStream &encodeDynamicCollection(
        const std::ranges::sized_range auto &collection) {
      *this << CompactInteger{collection.size()};
      for (const auto &item : collection) {
        *this << item;
      }
      return *this;
    }

    /**
     * @brief scale-encodes any static (fixed-size) collection
     * @param collection encoding collection
     * @return reference to stream
     */
    ScaleEncoderStream &encodeStaticCollection(
        const StaticCollection auto &collection) {
      for (const auto &item : collection) {
        *this << item;
      }
      return *this;
    }

    /**
     * @brief puts a byte to buffer
     * @param v byte value
     * @return reference to stream
     */
    ScaleEncoderStream &putByte(uint8_t v);

   private:
    ScaleEncoderStream &encodeOptionalBool(const std::optional<bool> &v);

    const bool drop_data_ = false;
    std::deque<uint8_t> stream_;
    size_t bytes_written_ = 0;
  };

  /**
   * @brief scale-encodes any enum type as its underlying type
   * Defined outside ScaleEncoderStream to allow custom overloads for
   * specific enum types.
   * @tparam T enum type
   * @param v value of the enum type
   * @return reference to stream
   */
  template <typename S,
            typename T,
            typename E = std::decay_t<T>,
            typename = std::enable_if_t<S::is_encoder_stream>,
            typename = std::enable_if_t<std::is_enum_v<E>>>
  S &operator<<(S &s, const T &v) {
    return s << static_cast<std::underlying_type_t<E>>(v);
  }

}  // namespace scale