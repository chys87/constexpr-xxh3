constexpr-xxh3
==============

This is a C++20 constexpr implementation of the XXH3 64-bit variant of [xxHash](https://github.com/Cyan4973/xxHash)

Three functions are implemented: `XXH3_64bits_const`, `XXH3_64bits_withSecret_const`, `XXH3_64bits_withSeed_const`.

Also included is a unit test to ensure they produce exactly the same results as the xxHash library.

Credits to:

* @Cyan4973 -- author of [xxHash](https://github.com/Cyan4973/xxHash)
* @ekpyron -- author of [xxhashct](https://github.com/ekpyron/xxhashct), a constexpr implementation of XXH64 and XXH32
* @t-mat -- for sharing [another constexpr implementation of XXH32](https://github.com/Cyan4973/xxHash/issues/496)


## Usage

### Basic interfaces

Basic interfaces mimic upstream interfaces.

* `XXH3_64bits_const(const T* input, size_t len)`: Hashes a series of bytes
* `XXH3_64bits_withSecret_const(const T* input, size_t len, const S* secret, size_t secretSize)`:
  Hashes a series of bytes, using user-provided secret.
* `XXH3_64bits_withSeed_const(const T* input, size_t len, uint64_t seed)`:
  Hashes a series of bytes, using user-provided seed.

Types `T` and `S` can be any of the following:

* `char`
* `signed char` (a.k.a. `int8_t`)
* `unsigned char` (a.k.a `uint8_t`)
* `char8_t`
* `std::byte`

We are unable to provide a constexpr interface with parameter type
`const void*` because of limitation imposed by the C++ standard, which
apparently does not want constexpr evaluation to depend on byte
representation.  This means that we cannot directly hash,
for example, a structure, at compile time, unless we explicitly copy it
member-wise to an array of bytes and manually take care of
memory layout (including padding) and endianness.

### Convenient interfaces

In a constexpr context, it is often more convenient to be able to pass the
input bytes and length as one parameter.  This is what the convenient
interfaces provide:

* `XXH3_64bits_const(const Bytes& input)`
* `XXH3_64bits_withSecret_const(const Bytes& input, const Bytes& secret)`
* `XXH3_64bits_withSeed_const(const Bytes& input, uint64_t seed)`

`Bytes` can be any of the following types:

* String literal type, including conventional `"string"` and UTF-8 `u8"string"`
* An object type that is “like” a string or byte array, e.g.:
    * `std::string_view`
    * `std::u8string_view`
    * `std::span<const char>`
    * `std::array<char, N>`

Note that null bytes embedded in string literals are considered part of the
string, e.g. `XXH3_64bits_const("a\0b")` is equivalent to
`XXH3_64bits_const("a\0b", 3)` rather than `XXH3_64bits_const("a", 1)`.

Unfortunately, we cannot distinguish a string literal from a “real”
`const char[]`.  This means the following code snippet hashes 3 bytes instead
of 4.

```c++
constexpr char bytes[] = {0xff, 0xfc, 0xfb, 0xfa};
constexpr uint64_t hash = XXH3_64bits_const(bytes);
```

This is unfortunate, but there appears to be no simple way to work it around.
For now, we have to use `XXH3_64bits_const(bytes, sizeof(bytes))`
or `XXH3_64bits_const(std::span(bytes))` in this case.

## TODO

Implement the 128-bit version
