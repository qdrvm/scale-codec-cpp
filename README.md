# SCALE codec C++ implementation
**SCALE (Simple Concatenated Aggregate Little-Endian) is a lightweight serialization format commonly used in blockchain applications.**

It allows encoding and decoding following data types:
* Built-in integer types specified by size:
  * ```uint8_t```
  * ```uint16_t```
  * ```uint32_t```
  * ```uint64_t```
* Multiprecision integer from boost:
  * ```uint128_t```
  * ```uint256_t```
  * ```uint512_t```
  * ```uint1024_t```
* boolean values
* pairs, tuples and other structurally bindable types (limited by N members)
* aggregates limited by N field (except array, that coded as collection)
* compact integers represented by CompactInteger type (classic ans JAM-compatible)
* optional values represented by ```std::optional<T>``` and ```boost::optional<T>```
  * as special case of optional values ```*::optional<bool>``` is encoded using one byte following specification.
* various collections of items
  * if item codable
  * encodes item by item in order of forward iterator
  * decodes items by inserting in order as encoded
* variants represented by ```std::variant<T...>``` and ```boost::variant<T...>```

## encode(value, encoder)
It is function is in charge of encoding of value and store to backend set in encoder

## decode(value, encoder)
It is function is in charge of read and decode data from backend set in encoder and initialize provided variable

## Encoder 
class Encoder is in charge of encoding data
It receives values over `encode()` function and store encoded data into EncoderBackend
Additionally it receive values over `<<` operator.

## Decoder 
class Decoder is in charge of decoding data
It initializes provided value over `decode()` function by decoded value from encoded source data
Additionally it initialize provided values over `>>` operator.

## ScaleEncoder
concept ScaleEncoder covers any implementation of Encoder (differing by EncoderBackend type)

## ScaleDecoder
concept ScaleDecoder covers any implementation of Decoder (differing by DecoderBackend type)

## Example 
```c++
Encoder<ToBytes> encoder; // Encoder is used backend 'to bytes'

uint32_t ui32 = 123u;
uint8_t ui8 = 234u;
std::string str = "asdasdasd";
auto * raw_str = "zxczxczx";
bool b = true;
CompactInteger ci = 123456789;
boost::variant<uint8_t, uint32_t, CompactInteger> vint = CompactInteger(12345);
std::optional<std::string> opt_str = "asdfghjkl";
std::optional<bool> opt_bool = false;
std::pair<uint8_t, uint32_t> pair{1u, 2u};
std::vector<uint32_t> coll_ui32 = {1u, 2u, 3u, 4u};
std::vector<std::string> coll_str = {"asd", "fgh", "jkl"};
std::vector<std::vector<int32_t>> coll_coll_i32 = {{1, 2, 3}, {4, 5, 6, 7}};

try {
  // functional style
  encode(ui32, encoder); 
  encode(ui8, encoder); 
  encode(str, encoder); 
  // combine for one call
  encode(std::tie(raw_str, b, ci, vint), encoder);
  // stream-style
  encoder << opt_str << opt_bool << pair << coll_ui32 << coll_str << coll_coll_i32;
} catch (std::runtime_error &e) {
  // handle error
  // for example make and return outcome::result
  return outcome::failure(e.code());
}
```
You can now get encoded data:
```c++
ByteArray data = encoder.backend().to_vector();
```
Now you can decode that data back:
```c++
Decoder<FromBytes> decoder(data); // Decoder is used backend 'from bytes'

uint32_t ui32 = 0u;
uint8_t ui8 = 0u;
std::string str;
bool b = true;
CompactInteger ci;
boost::variant<uint8_t, uint32_t, CompactInteger> vint;
std::optional<std::string> opt_str;
std::optional<bool> opt_bool;
std::pair<uint8_t, uint32_t> pair{};
std::vector<uint32_t> coll_ui32;
std::vector<std::string> coll_str;
std::vector<std::vector<int32_t>> coll_coll_i32;
try {
  // functional style
  decode(ui32, decoder); 
  decode(ui8, decoder); 
  decode(str, decoder); 
  // combine for one call
  decode(std::tie(raw_str, b, ci, vint), decoder);
  // stream-style
  decoder >> opt_str >> opt_bool >> pair >> coll_ui32 >> coll_str >> coll_coll_i32;
} catch (std::system_error &e) {
  // handle error
}
```
Now we have variables initialized by decoded values

## Custom types
You may need to encode or decode custom data types, you have to define custom `encode()` and `decode()` function.
Please note, that your custom data types must be default-constructible.
```c++
struct MyType {
    int a = 0;
    std::string b;
    
    friend void encode(const MyType &v, ScaleEncoder auto &encoder) {
      encoder << a;
      encode(b, encoder);
    }
    friend void decode(MyType &v, ScaleDecoder auto &decoder) {
      decoder >> a;
      decode(b, decoder);
    }
};
```
Now you can use them in collections, optionals and variants
```c++
std::vector<MyType> src_vec = {{1, "asd"}, {2, "qwe"}};
try { 
  encode << src_vec;
} catch (...) {
  // handle error
}
ByteArray data = encoder.backend().to_vector();

Decoder<FromBytes> decoder(data);
std::vector<MyType> dst_vec;
try {
  decode(dst, decoder);
} catch (...) {
  // handle error
}
```

## Convenience functions
Library provides ready well done function to encode/decode in one line. You should just use import it in you namespace: 

```c++
// template <typename T> 
// outcome::result<std::vector<uint8_t>> encode(T &&value);
using ::scale::impl::bytes::encode;

SomeClass object = {...};
auto res = encode(object); // <- Just one-line call
if (res.has_value()) {
  std::vector<uint8_t> encoded = std::move(res.value()); // Bytes of encoded data
}

// template <typename T> 
// outcome::result<T> decode(const RangeOfBytes auto& span);
using ::scale::impl::memory::decode;

BytesArray data = {...};
auto res = decode<SomeClass>(data); // <- Just one-line call
if (res.has_value()) {
  SomeClass object = std::move(res.value()); // Decoded value
}

// using Encoder = ::scale::Encoder<::scale::backend::ToBytes>;
using ::scale::impl::bytes::Encoder;

SomeClass object = {...};
Encoder encoder;
try {
    encoder << object;
    // or encode(object, decoder);
    std::vector<uint8_t> encoded = std::move(res.value())
} catch ...

// using Decoder = ::scale::Decoder<::scale::backend::FromBytes>;
using ::scale::impl::bytes::Decoder;

BytesArray data = {...};
Decoder decoder(data);
try {
    Object object;
    decoder >> object;
    // or decode(object, decoder);
} catch ...

```
