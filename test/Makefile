COMMON_CXXFLAGS := -DXXH_INLINE_ALL -O2 -Wall -Wextra -std=c++20 -I..

CXX := g++
CXXFLAGS = $(COMMON_CXXFLAGS) -fmax-errors=3

CLANGXX := clang++
CLANGXXFLAGS := $(COMMON_CXXFLAGS)

LIBS := -lgtest_main -lgtest -lpthread

.PHONY: test-gcc test-clang clean

all: test-gcc test-clang
test-gcc: constexpr-xxh3-test-gcc
	./$<

test-clang: constexpr-xxh3-test-clang
	./$<

constexpr-xxh3-test-gcc: constexpr-xxh3-test.cc ../constexpr-xxh3.h
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

constexpr-xxh3-test-clang: constexpr-xxh3-test.cc ../constexpr-xxh3.h
	$(CLANGXX) $(CLANGXXFLAGS) -o $@ $< $(LIBS)

clean:
	rm -f constexpr-xxh3-test-gcc constexpr-xxh3-test-clang
