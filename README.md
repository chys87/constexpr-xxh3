constexpr-xxh3
==============

This is a C++20 constexpr implementation of the XXH3 64-bit variant of [xxHash](https://github.com/Cyan4973/xxHash)

Three functions are implemented: `XXH3_64bits_const`, `XXH3_64bits_withSecret_const`, `XXH3_64bits_withSeed_const`.

Also included is a unit test to ensure they produce exactly the same results as the xxHash library.

Credits to:

* @Cyan4973 -- author of [xxHash](https://github.com/Cyan4973/xxHash)
* @ekpyron -- author of [xxhashct](https://github.com/ekpyron/xxhashct), a constexpr implementation of XXH64 and XXH32
* @t-mat -- for sharing [another constexpr implementation of XXH32](https://github.com/Cyan4973/xxHash/issues/496)
