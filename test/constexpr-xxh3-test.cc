#include "constexpr-xxh3.h"

#include <cstddef>
#include <memory>
#if __has_include(<span>)
#include <span>
#endif
#include <string_view>

#include "gtest/gtest.h"
#include "xxhash.h"

#ifndef CONSTEXPR_XXH3_ALLOW_CLASS_REPR
#error "CONSTEXPR_XXH3_ALLOW_CLASS_REPR macro modifier needs to be defined to test the full API."
#endif

namespace {

using namespace constexpr_xxh3;

template <size_t N>
inline constexpr std::array<int8_t, N> MakeBytes() {
  std::array<int8_t, N> res{};
  for (std::size_t i = 0; i < N; ++i) {
    res[i] = int8_t(i * 127);
  }
  return res;
}

template <size_t N, typename Callback>
inline constexpr void IterateSkippedIndices(Callback cb) {
  size_t i = 0;
  while (i < N) {
    cb(i);
    if (i <= 240) {
      ++i;
    } else {
      i += i / 3;
    }
  }
}

template <size_t N>
inline constexpr size_t GetSkippedIndicesCount() {
  size_t r = 0;
  IterateSkippedIndices<N>([&r](size_t) constexpr { ++r; });
  return r;
}

template <size_t N>
inline constexpr auto MakeSkippedIndices() {
  std::array<size_t, GetSkippedIndicesCount<N>()> res{};
  size_t i = 0;
  IterateSkippedIndices<N>([&](size_t k) constexpr { res[i++] = k; });
  return res;
}

template <size_t K, typename ConstexprRun>
consteval std::array<uint64_t, K> ConstexprEvaluate(std::array<size_t, K> idx,
                                                    ConstexprRun run) {
  std::array<uint64_t, K> res{};
  for (size_t k = 0; k < K; ++k) res[k] = run(idx[k]);
  return res;
}

template <size_t K, typename RegularRun>
void AssertAll(std::array<size_t, K> idx,
               std::array<uint64_t, K> constexpr_result, RegularRun rr) {
  for (size_t k = 0; k < K; ++k) {
    uint64_t const_res = constexpr_result[k];
    uint64_t non_const_res = rr(idx[k]);
    ASSERT_EQ(const_res, non_const_res)
        << "length=" << idx[k] << " constexpr=" << const_res
        << " non-constexpr=" << non_const_res;
  }
}

constexpr size_t kN = 4096;
constexpr auto kIdx = MakeSkippedIndices<kN>();
constexpr auto kBytes = MakeBytes<kN>();
constexpr auto kSecret = MakeBytes<256>();

TEST(ConstexprXXH3Test, BasicCompilabilityTest) {
  // At least these types must be supported:
  // char, signed char, unsigned char, char8_t, std::byte

  constexpr signed char kSignedChars[] = "Hello";
  constexpr unsigned char kUnsignedChars[] = "Hello";
#if defined __cpp_lib_byte && __cpp_lib_byte >= 201603
  constexpr std::byte kStdBytes[]{std::byte('H'), std::byte('e'),
                                  std::byte('l'), std::byte('l'),
                                  std::byte('o')};
#endif


  {
    constexpr uint64_t kRefValue = XXH3_64bits_const("Hello", 5);
#if defined __cpp_char8_t && __cpp_char8_t >= 201811
    static_assert(XXH3_64bits_const(u8"Hello", 5) == kRefValue);
#endif
    static_assert(XXH3_64bits_const(kSignedChars, 5) == kRefValue);
    static_assert(XXH3_64bits_const(kUnsignedChars, 5) == kRefValue);
#if defined __cpp_lib_byte && __cpp_lib_byte >= 201603
    static_assert(XXH3_64bits_const(kStdBytes, 5) == kRefValue);
#endif
  }

  {
    constexpr uint64_t kRefValue = XXH3_64bits_withSecret_const(
        "Hello", 5, kSecret.data(), kSecret.size());
#if defined __cpp_char8_t && __cpp_char8_t >= 201811
    static_assert(XXH3_64bits_withSecret_const(u8"Hello", 5, kSecret.data(),
                                               kSecret.size()) == kRefValue);
#endif
    static_assert(XXH3_64bits_withSecret_const(kSignedChars, 5, kSecret.data(),
                                               kSecret.size()) == kRefValue);
    static_assert(XXH3_64bits_withSecret_const(kUnsignedChars, 5,
                                               kSecret.data(),
                                               kSecret.size()) == kRefValue);
#if defined __cpp_lib_byte && __cpp_lib_byte >= 201603
    static_assert(XXH3_64bits_withSecret_const(kStdBytes, 5, kSecret.data(),
                                               kSecret.size()) == kRefValue);
#endif
  }

  {
    constexpr uint64_t kRefValue = XXH3_64bits_withSeed_const("Hello", 5, 1);
#if defined __cpp_char8_t && __cpp_char8_t >= 201811
    static_assert(XXH3_64bits_withSeed_const(u8"Hello", 5, 1) == kRefValue);
#endif
    static_assert(XXH3_64bits_withSeed_const(kSignedChars, 5, 1) == kRefValue);
    static_assert(XXH3_64bits_withSeed_const(kUnsignedChars, 5, 1) ==
                  kRefValue);
#if defined __cpp_lib_byte && __cpp_lib_byte >= 201603
    static_assert(XXH3_64bits_withSeed_const(kStdBytes, 5, 1) == kRefValue);
#endif
  }
}

TEST(ConstexprXXH3Test, ConvenienceInterfaceTest) {
  constexpr uint64_t kRefValue = XXH3_64bits_const("Hello", 5);

  // String literal
  static_assert(XXH3_64bits_const("Hello") == kRefValue);
#if defined __cpp_char8_t && __cpp_char8_t >= 201811
  static_assert(XXH3_64bits_const(u8"Hello") == kRefValue);
#endif

  // Explicit const ByteType[]
  {
    constexpr char kBytes[]{"Hello"};
    static_assert(XXH3_64bits_const(kBytes) == kRefValue);
  }
  {
    constexpr signed char kBytes[]{"Hello"};
    static_assert(XXH3_64bits_const(kBytes) == kRefValue);
  }
#if defined __cpp_lib_byte && __cpp_lib_byte >= 201603
  {
    constexpr std::byte kBytes[]{std::byte('H'), std::byte('e'), std::byte('l'),
                                 std::byte('l'), std::byte('o')};
    static_assert(XXH3_64bits_const(kBytes) == kRefValue);
  }
#endif

  // std::string_view, std::u8string_view
  static_assert(XXH3_64bits_const(std::string_view("Hello")) == kRefValue);
#if defined __cpp_char8_t && __cpp_char8_t >= 201811
  static_assert(XXH3_64bits_const(std::u8string_view(u8"Hello")) == kRefValue);
#endif

  // std::span
#ifdef __cpp_lib_span
  {
    constexpr char kBytes[5]{'H', 'e', 'l', 'l', 'o'};
    static_assert(XXH3_64bits_const(std::span(kBytes)) == kRefValue);
    static_assert(XXH3_64bits_const(std::span<const char, std::dynamic_extent>(
                      kBytes, sizeof(kBytes))) == kRefValue);
  }
#endif

  // std::array
  {
    constexpr std::array<char, 5> kBytes{'H', 'e', 'l', 'l', 'o'};
    static_assert(XXH3_64bits_const(std::span(kBytes)) == kRefValue);
  }

  // withSecret
  static_assert(
      XXH3_64bits_withSecret_const("Hello", kSecret) ==
      XXH3_64bits_withSecret_const("Hello", 5, kSecret.data(), kSecret.size()));

  // withSeed
  static_assert(XXH3_64bits_withSeed_const("Hello", 2554) ==
                XXH3_64bits_withSeed_const("Hello", 5, 2554));
}

TEST(ConstexprXXH3Test, ObjectRepresentationTest) {
  // Tests are done at runtime, because you can't feed these inputs into the
  // basic interface at compile-time without either hardcoding the results or
  // using std::bit_cast again, which would defeat the purpose of this test.

  constexpr uint64_t kSingleIntInput = 42;
  constexpr double kSingleFloatInput = 3.141592653589793;

  struct ClassTestMock
  {
    uint64_t integer;
    double floating_point;
  };
  constexpr ClassTestMock kSingleClassInput = {kSingleIntInput,
                                               kSingleFloatInput};
                                               
  ASSERT_EQ(XXH3_64bits_const(kSingleIntInput),
            XXH3_64bits(&kSingleIntInput, sizeof(kSingleIntInput)));
  ASSERT_EQ(XXH3_64bits_const(kSingleFloatInput),
            XXH3_64bits(&kSingleFloatInput, sizeof(kSingleFloatInput)));
  ASSERT_EQ(XXH3_64bits_const(kSingleClassInput),
            XXH3_64bits(&kSingleClassInput, sizeof(kSingleClassInput)));

  // withSecret
  
  ASSERT_EQ(XXH3_64bits_withSecret_const(kSingleIntInput, kSecret),
            XXH3_64bits_withSecret(&kSingleIntInput, sizeof(kSingleIntInput),
                                   kSecret.data(), kSecret.size()));
  ASSERT_EQ(
      XXH3_64bits_withSecret_const(kSingleFloatInput, kSecret),
      XXH3_64bits_withSecret(&kSingleFloatInput, sizeof(kSingleFloatInput),
                             kSecret.data(), kSecret.size()));
  ASSERT_EQ(
      XXH3_64bits_withSecret_const(kSingleClassInput, kSecret),
      XXH3_64bits_withSecret(&kSingleClassInput, sizeof(kSingleClassInput),
                             kSecret.data(), kSecret.size()));

  // withSeed

  constexpr uint64_t kSeed = 0x2554;

  ASSERT_EQ(XXH3_64bits_withSeed_const(kSingleIntInput, kSeed),
            XXH3_64bits_withSeed(&kSingleIntInput, sizeof(kSingleIntInput),
                                 kSeed));
  ASSERT_EQ(XXH3_64bits_withSeed_const(kSingleFloatInput, kSeed),
            XXH3_64bits_withSeed(&kSingleFloatInput, sizeof(kSingleFloatInput),
                                 kSeed));
  ASSERT_EQ(XXH3_64bits_withSeed_const(kSingleClassInput, kSeed),
            XXH3_64bits_withSeed(&kSingleClassInput, sizeof(kSingleClassInput),
                                 kSeed));

  constexpr std::array<ClassTestMock, 2> kMultipleClassInput = {
      kSingleClassInput, kSingleClassInput};

  // Arrays of objects
  ASSERT_EQ(
      XXH3_64bits_const(kMultipleClassInput),
      XXH3_64bits(kMultipleClassInput.data(), sizeof(kMultipleClassInput)));

  // Arrays with secret
  ASSERT_EQ(XXH3_64bits_withSecret_const(kMultipleClassInput, kSecret),
            XXH3_64bits_withSecret(kMultipleClassInput.data(),
                                   sizeof(kMultipleClassInput), kSecret.data(),
                                   kSecret.size()));

  // Arrays with seed
  ASSERT_EQ(XXH3_64bits_withSeed_const(kMultipleClassInput, kSeed),
            XXH3_64bits_withSeed(kMultipleClassInput.data(),
                                 sizeof(kMultipleClassInput), kSeed));
}

TEST(ConstexprXXH3Test, Simple) {
  AssertAll(kIdx,
            ConstexprEvaluate(
                kIdx, [](size_t n) consteval noexcept {
                  return XXH3_64bits_const(kBytes.data(), n);
                }),
            [](size_t n) noexcept { return XXH3_64bits(kBytes.data(), n); });
}

TEST(ConstexprXXH3Test, Seeded) {
  constexpr uint64_t kSeed = 0x2554;
  AssertAll(kIdx,
            ConstexprEvaluate(
                kIdx,
                [&](size_t n) consteval noexcept {
                  return XXH3_64bits_withSeed_const(kBytes.data(), n, kSeed);
                }),
            [&](size_t n) noexcept {
              return XXH3_64bits_withSeed(kBytes.data(), n, kSeed);
            });
}

TEST(ConstexprXXH3Test, Seeded0) {
  constexpr uint64_t kSeed = 0;
  AssertAll(kIdx,
            ConstexprEvaluate(
                kIdx,
                [&](size_t n) consteval noexcept {
                  return XXH3_64bits_withSeed_const(kBytes.data(), n, kSeed);
                }),
            [&](size_t n) noexcept {
              return XXH3_64bits_withSeed(kBytes.data(), n, kSeed);
            });
}

TEST(ConstexprXXH3Test, WithSecret) {
  AssertAll(kIdx,
            ConstexprEvaluate(
                kIdx, [](size_t n) consteval noexcept {
                  return XXH3_64bits_withSecret_const(
                      kBytes.data(), n, kSecret.data(), kSecret.size());
                }),
            [](size_t n) noexcept {
              return XXH3_64bits_withSecret(kBytes.data(), n, kSecret.data(),
                                            kSecret.size());
            });
}

}  // namespace
