

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

The functions are in the `constexpr_xxh3` namespace.

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

### Hashing object representations

The library also provides interfaces for hashing the representation of an
object directly, such as `int`, `float` and even `struct`/`class`:

* `XXH3_64bits_const(const T& input)`
* `XXH3_64bits_withSecret_const(const T& input, const Secret& secret)`
* `XXH3_64bits_withSeed_const(const T& input, uint64_t seed)`

For arrays of hashable objects, corresponding overloads are also provided:

* `XXH3_64bits_const(const T (&input)[N])`
* `XXH3_64bits_withSecret_const(const T (&input)[N], const Secret& secret)`
* `XXH3_64bits_withSeed_const(const T (&input)[N], uint64_t seed)`

For an object to be hashable through this interface, it must:
* be trivially copyable;
* not be a union type;
* not be a pointer type;
* not be a pointer to member type;
* not be a volatile-qualified type;
* not be a structure/class (unless explicitly allowed).

Keep in mind that this interface performs a copy of the representation of the
object/array about to be hashed, which introduce a usually negligible overhead
for the intended use cases.

#### For structures/classes

Those objects are a special edge case, as they are disabled by default because
they can be a safety hazard.

Indeed, these interfaces operate on the object's representation rather than its
semantics, so the resulting hashes are not guaranteed to be stable, due, for
example, to inserted padding and pointer fields.

Padding can even cause the compiler to reject the code because it may contain
indeterminate values, especially on GCC. Therefore, one must be aware of all
these potential caveats before attempting to hash structures.

The recommended approach is not to enable this functionality, but rather to
create your own semantically meaningful, per-field hasher using the available
interfaces.

To allow structures and classes in the aforementioned interfaces, define
`CONSTEXPR_XXH3_ALLOW_CLASS_REPR`.

## TODO

Implement the 128-bit version
