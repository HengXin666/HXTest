#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}

// EBO
#include <type_traits>

namespace hx {

template <typename T, typename = void>
struct EboImpl {
  T& get() noexcept {
    return data_;
  }

  T data_;
};

template <typename T>
struct EboImpl<T, std::void_t<std::is_class<T>>> : T {
  T& get() noexcept {
    return *this;
  }
};

template <typename T, typename... Ts>
struct Ebo : EboImpl<T>, Ebo<Ts>... {};

template <typename T>
struct Ebo<T> : EboImpl<T> {};

/// === tests ===

struct A {};
struct B { [[maybe_unused]] int a_; };
class C {};

static_assert(sizeof(A) == 1);
static_assert(sizeof(B) == sizeof(int));

// ebo
static_assert(sizeof(Ebo<A>) == 1);
static_assert(sizeof(Ebo<A, C>) == 1);
static_assert(sizeof(Ebo<A, B, C>) == sizeof(B));

// libc++
static_assert(sizeof(std::tuple<A, B, C>) == sizeof(B));

} // namespace hx