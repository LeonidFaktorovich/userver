#include <utils/optional_ref.hpp>

#include <gtest/gtest.h>

TEST(OptionalRef, Constructions) {
  using utils::OptionalRef;

  static_assert(std::is_constructible_v<OptionalRef<int>, int&>);
  static_assert(std::is_constructible_v<OptionalRef<const int>, int&>);
  static_assert(std::is_constructible_v<OptionalRef<const int>, const int&>);

  static_assert(!std::is_constructible_v<OptionalRef<int>, const int&>);
  static_assert(!std::is_constructible_v<OptionalRef<int>, int>);
  static_assert(!std::is_constructible_v<OptionalRef<int>, int&&>);

  static_assert(!std::is_constructible_v<OptionalRef<int>, float&>);
  static_assert(!std::is_constructible_v<OptionalRef<float>, int&>);
  static_assert(!std::is_constructible_v<OptionalRef<int>, short&>);
  static_assert(!std::is_constructible_v<OptionalRef<short>, int&>);
}

TEST(OptionalRef, Values) {
  int a1_val = 1;
  int b1_val = 1;
  int b2_val = 2;

  utils::OptionalRef<int> a1(a1_val), b1(b1_val), b2(b2_val);
  EXPECT_TRUE(a1 == b1);
  EXPECT_FALSE(a1 != b1);

  EXPECT_FALSE(a1 == b2);
  EXPECT_TRUE(a1 != b2);

  utils::OptionalRef<int> def;
  EXPECT_FALSE(a1 == def);
  EXPECT_TRUE(a1 != def);
  EXPECT_FALSE(b2 == def);
  EXPECT_TRUE(b2 != def);
}

TEST(OptionalRef, ConstValues) {
  int a1_val = 1;
  const int b1_val = 1;
  int b2_val = 2;

  utils::OptionalRef<const int> a1(a1_val), b1(b1_val), b2(b2_val);
  EXPECT_TRUE(a1 == b1);
  EXPECT_FALSE(a1 != b1);

  EXPECT_FALSE(a1 == b2);
  EXPECT_TRUE(a1 != b2);

  utils::OptionalRef<const int> def;
  EXPECT_FALSE(a1 == def);
  EXPECT_TRUE(a1 != def);
  EXPECT_FALSE(b2 == def);
  EXPECT_TRUE(b2 != def);
}

TEST(OptionalRef, OptionalValues) {
  std::optional<int> a1_val = 1;
  std::optional<int> b1_val = 1;
  std::optional<int> b2_val = 2;

  utils::OptionalRef<int> a1(a1_val), b1(b1_val), b2(b2_val);
  EXPECT_TRUE(a1 == b1);
  EXPECT_FALSE(a1 != b1);

  EXPECT_FALSE(a1 == b2);
  EXPECT_TRUE(a1 != b2);

  utils::OptionalRef<int> def;
  EXPECT_FALSE(a1 == def);
  EXPECT_TRUE(a1 != def);
  EXPECT_FALSE(b2 == def);
  EXPECT_TRUE(b2 != def);
}

TEST(OptionalRef, ConstOptionalValues) {
  std::optional<int> a1_val = 1;
  std::optional<const int> b1_val = 1;
  const std::optional<int> b2_val = 2;

  utils::OptionalRef<const int> a1(a1_val), b1(b1_val), b2(b2_val);
  EXPECT_TRUE(a1 == b1);
  EXPECT_FALSE(a1 != b1);

  EXPECT_FALSE(a1 == b2);
  EXPECT_TRUE(a1 != b2);

  utils::OptionalRef<const int> def;
  EXPECT_FALSE(a1 == def);
  EXPECT_TRUE(a1 != def);
  EXPECT_FALSE(b2 == def);
  EXPECT_TRUE(b2 != def);
}

TEST(OptionalRef, BaseDerived) {
  struct Base {
    bool operator==(const Base& other) const { return this == &other; }
  };
  struct Derived : Base {};

  Derived derived1, derived2;

  utils::OptionalRef<const Base> cb1(derived1), cb2(derived2);
  utils::OptionalRef<Base> b1(derived1);

  EXPECT_TRUE(cb1 != cb2);
  EXPECT_TRUE(cb1 == b1);
  EXPECT_TRUE(cb2 != b1);
}