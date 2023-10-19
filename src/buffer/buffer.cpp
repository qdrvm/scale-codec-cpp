/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <scale/buffer/buffer.hpp>

#include <iostream>

#include <scale/buffer/hexutil.hpp>

namespace scale {

  size_t Buffer::size() const {
    return data_.size();
  }

  Buffer &Buffer::putUint32(uint32_t n) {
    data_.push_back(static_cast<unsigned char &&>((n >> 24) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 16) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 8) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n)&0xFF));

    return *this;
  }

  Buffer &Buffer::putUint64(uint64_t n) {
    data_.push_back(static_cast<unsigned char &&>((n >> 56u) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 48u) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 40u) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 32u) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 24u) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 16u) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n >> 8u) & 0xFF));
    data_.push_back(static_cast<unsigned char &&>((n)&0xFF));

    return *this;
  }

  std::string Buffer::toHex() const {
    return hex_lower(data_);
  }

  std::string_view Buffer::toString() const {
    return {reinterpret_cast<const char *>(data_.data()),  // NOLINT
            data_.size()};
  }

  bool Buffer::empty() const {
    return data_.empty();
  }

  Buffer::Buffer(std::initializer_list<uint8_t> il) : data_(il) {}

  Buffer::iterator Buffer::begin() {
    return data_.begin();
  }

  Buffer::iterator Buffer::end() {
    return data_.end();
  }

  Buffer &Buffer::putUint8(uint8_t n) {
    data_.push_back(n);
    return *this;
  }

  uint8_t Buffer::operator[](size_t index) const {
    return data_[index];
  }

  uint8_t &Buffer::operator[](size_t index) {
    return data_[index];
  }

  outcome::result<Buffer> Buffer::fromHex(std::string_view hex) {
    OUTCOME_TRY(bytes, unhex(hex));
    return Buffer{std::move(bytes)};
  }

  Buffer::Buffer(std::vector<uint8_t> vector) : data_(std::move(vector)) {}
  Buffer::Buffer(const RangeOfBytes auto &range)
      : data_(range.begin(), range.end()) {}

  const std::vector<uint8_t> &Buffer::toVector() const {
    return data_;
  }

  Buffer::const_iterator Buffer::begin() const {
    return data_.begin();
  }

  Buffer::const_iterator Buffer::end() const {
    return data_.end();
  }

  const uint8_t *Buffer::data() const {
    return data_.data();
  }

  uint8_t *Buffer::data() {
    return data_.data();
  }

  Buffer::Buffer(size_t size, uint8_t byte) : data_(size, byte) {}

  bool Buffer::operator==(const RangeOfBytes auto &other) const noexcept {
    return std::equal(data_.begin(), data_.end(), other.begin(), other.end());
  }

  bool Buffer::operator<(const RangeOfBytes auto &other) const noexcept {
    return std::lexicographical_compare(
        begin(), end(), other.begin(), other.end());
  }

  template <typename T>
  Buffer &Buffer::putRange(const T &begin, const T &end) {
    static_assert(sizeof(*begin) == 1);
    data_.insert(std::end(data_), begin, end);
    return *this;
  }

  Buffer &Buffer::put(const RangeOfBytes auto &range) {
    return putRange(range.begin(), range.end());
  }

  Buffer &Buffer::putBytes(const uint8_t *begin, const uint8_t *end) {
    return putRange(begin, end);
  }

  Buffer &Buffer::putBuffer(const Buffer &buf) {
    return put(buf.toVector());
  }

  void Buffer::clear() {
    data_.clear();
  }

  Buffer::Buffer(const uint8_t *begin, const uint8_t *end)
      : data_{begin, end} {}

  std::vector<uint8_t> &Buffer::toVector() {
    return data_;
  }

  Buffer &Buffer::reserve(size_t size) {
    data_.reserve(size);
    return *this;
  }

  Buffer &Buffer::resize(size_t size) {
    data_.resize(size);
    return *this;
  }

  std::string Buffer::asString() const {
    return std::string{data_.begin(), data_.end()};
  }

  outcome::result<Buffer> Buffer::fromString(const std::string &src) {
    return Buffer({src.begin(), src.end()});
  }

  Buffer Buffer::subbuffer(size_t offset, size_t length) const {
    return Buffer(ConstSpanOfBytes(*this).subspan(offset, length));
  }

  Buffer &Buffer::operator+=(const Buffer &other) noexcept {
    return this->put(other);
  }

  std::ostream &operator<<(std::ostream &os, const Buffer &buffer) {
    return os << buffer.toHex();
  }

}  // namespace scale
