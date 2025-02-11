/**
 * Copyright Quadrivium LLC
 * All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <qtils/empty.hpp>
#include <qtils/test/outcome.hpp>
#include <scale/definitions.hpp>
#include <scale/scale.hpp>

#ifndef CUSTOM_CONFIG_ENABLED
#error \
    "This file should not be compiled, because custom config support is not enabed"
#endif

using scale::as_compact;
using scale::ByteArray;
using scale::Compact;
using scale::ConstSpanOfBytes;
using scale::impl::with_buffer::decode;
using scale::impl::with_buffer::encode;
using scale::Decoder;
using scale::Encoder;
using scale::ScaleDecoder;
using scale::ScaleEncoder;
using scale::backend::FromBytes;
using scale::backend::ToBytes;

template <typename T, typename... Configs>
outcome::result<std::vector<uint8_t>> encode_with_config(T &&value,
                                                         Configs &&...configs) {
  Encoder<ToBytes> encoder(std::forward<Configs>(configs)...);
  try {
    encode(std::forward<T>(value), encoder);
  } catch (std::system_error &e) {
    return outcome::failure(e.code());
  }
  return std::move(encoder).backend().to_vector();
}

template <typename T, typename... Configs>
outcome::result<T> decode_with_config(ConstSpanOfBytes bytes,
                                      Configs &&...configs) {
  Decoder<FromBytes> decoder(bytes, std::forward<Configs>(configs)...);
  T value;
  try {
    decode(value, decoder);
  } catch (std::system_error &e) {
    return outcome::failure(e.code());
  }
  return std::move(value);
}

struct MulConfig {
  uint8_t multi;
};
struct AddConfig {
  uint8_t add;
};

struct Object : private qtils::Empty {
  std::vector<uint8_t> buff;

  bool operator==(const Object &) const = default;

  friend void encode(const Object &obj, ScaleEncoder auto &encoder) {
    auto mul = encoder.template getConfig<MulConfig>().multi;
    auto add = encoder.template getConfig<AddConfig>().add;

    encode(as_compact(obj.buff.size()), encoder);
    for (uint8_t i : obj.buff) {
      uint8_t x = i * mul + add;
      encoder.put(x);
    }
  }

  friend void decode(Object &obj, ScaleDecoder auto &decoder) {
    auto mul = decoder.template getConfig<MulConfig>().multi;
    auto add = decoder.template getConfig<AddConfig>().add;
    size_t size;
    decode(as_compact(size), decoder);
    obj.buff.resize(size);
    for (uint8_t &i : obj.buff) {
      uint8_t x;
      decoder >> x;
      i = (x - add) / mul;
    }
  }

 private:
  int x = 0;
};

TEST(CustomConfig, SunnyDayScenario) {
  MulConfig mul_two{2};
  MulConfig mul_three{3};
  AddConfig add_six{6};
  AddConfig add_twelve{12};

  // Encoding
  Object object;
  object.buff = {3, 6, 9};

  // X * 2 + 12
  ASSERT_OUTCOME_SUCCESS(  //
      encoded,
      encode_with_config(object, mul_two, add_twelve));

  // Check encoding
  {
    ASSERT_OUTCOME_SUCCESS(decoded, decode<std::vector<uint8_t>>(encoded));
    std::vector<uint8_t> expected = {18, 24, 30};
    ASSERT_EQ(decoded, expected) << "Incorrect encoded";
  }

  // Decode and check it
  {
    ASSERT_OUTCOME_SUCCESS(  //
        decoded,
        decode_with_config<Object>(encoded, mul_three, add_six));
    Object expected;
    expected.buff = {4, 6, 8};
    ASSERT_EQ(decoded, expected) << "Incorrect decoded";
  }
}
