#include <gtest/gtest.h>

#include <bits/tag_list.hpp>

namespace bits {

namespace {

struct Tag1 {};
struct Tag2 {};
struct Tag3 {};

using Tags0 = TagList<>;
using Tags1 = TagList<Tag1>;
using Tags2 = TagList<Tag1, Tag2>;
using Tags3 = TagList<Tag1, Tag2, Tag3>;

template <typename Tag>
using IsTag2 = std::is_same<Tag2, Tag>;

template <typename Tag>
struct Key;

template <>
struct Key<Tag1> {
  static constexpr auto value = 1;
};

template <>
struct Key<Tag2> {
  static constexpr auto value = 2;
};

template <>
struct Key<Tag3> {
  static constexpr auto value = 3;
};

template <typename Lhs, typename Rhs>
void assertSame() {
  static_assert(std::is_same<Lhs, Rhs>::value, "");
}

}  // namespace

TEST(TagListTest, SizeOf) {
  static_assert(sizeof(Tags0) == 1, "");
  static_assert(sizeof(Tags1) == 1, "");
  static_assert(sizeof(Tags2) == 1, "");
  static_assert(sizeof(Tags3) == 1, "");
}

TEST(TagListTest, HasTag) {
  static_assert(!hasTag<Tags0, Tag1>(), "");
  static_assert(!hasTag<Tags0, Tag2>(), "");
  static_assert(!hasTag<Tags0, Tag3>(), "");

  static_assert(hasTag<Tags1, Tag1>(), "");
  static_assert(!hasTag<Tags1, Tag2>(), "");
  static_assert(!hasTag<Tags1, Tag3>(), "");

  static_assert(hasTag<Tags2, Tag1>(), "");
  static_assert(hasTag<Tags2, Tag2>(), "");
  static_assert(!hasTag<Tags2, Tag3>(), "");

  static_assert(hasTag<Tags3, Tag1>(), "");
  static_assert(hasTag<Tags3, Tag2>(), "");
  static_assert(hasTag<Tags3, Tag3>(), "");
}

TEST(TagListTest, Reversed) {
  assertSame<Tags0, Reversed<Tags0>>();
  assertSame<TagList<Tag1>, Reversed<Tags1>>();
  assertSame<TagList<Tag2, Tag1>, Reversed<Tags2>>();
  assertSame<TagList<Tag3, Tag2, Tag1>, Reversed<Tags3>>();
}

TEST(TagListTest, Filtered) {
  assertSame<TagList<>, Filtered<Tags0, IsTag2>>();
  assertSame<TagList<>, Filtered<Tags1, IsTag2>>();
  assertSame<TagList<Tag2>, Filtered<Tags2, IsTag2>>();
  assertSame<TagList<Tag2>, Filtered<Tags2, IsTag2>>();
}

TEST(TagListTest, Concatenated) {
  assertSame<Tags0, Concatenated<Tags0, Tags0>>();
  assertSame<Tags1, Concatenated<Tags0, Tags1>>();
  assertSame<Tags2, Concatenated<Tags0, Tags2>>();
  assertSame<Tags3, Concatenated<Tags0, Tags3>>();
  assertSame<Tags1, Concatenated<Tags1, Tags0>>();
  assertSame<Tags2, Concatenated<Tags2, Tags0>>();
  assertSame<Tags3, Concatenated<Tags3, Tags0>>();
  assertSame<Tags3, Concatenated<Tags3, Tags0>>();
  assertSame<Tags3, Concatenated<TagList<Tag1>, TagList<Tag2, Tag3>>>();
}

TEST(TagListTest, Partitioned) {
  assertSame<Tags0, typename Partitioned<Tags0, IsTag2>::lhs>();
  assertSame<Tags0, typename Partitioned<Tags0, IsTag2>::rhs>();
  assertSame<Tags0, typename Partitioned<Tags1, IsTag2>::lhs>();
  assertSame<TagList<Tag1>, typename Partitioned<Tags1, IsTag2>::rhs>();
  assertSame<TagList<Tag2>, typename Partitioned<Tags2, IsTag2>::lhs>();
  assertSame<TagList<Tag1>, typename Partitioned<Tags2, IsTag2>::rhs>();
  assertSame<TagList<Tag2>, typename Partitioned<Tags3, IsTag2>::lhs>();
  assertSame<TagList<Tag1, Tag3>, typename Partitioned<Tags3, IsTag2>::rhs>();
}

TEST(TagListTest, Sorted) {
  assertSame<Tags0, Sorted<Tags0, Key>>();
  assertSame<Tags1, Sorted<TagList<Tag1>, Key>>();
  assertSame<Tags2, Sorted<TagList<Tag2, Tag1>, Key>>();
  assertSame<Tags3, Sorted<TagList<Tag3, Tag2, Tag1>, Key>>();
  assertSame<Tags3, Sorted<TagList<Tag2, Tag3, Tag1>, Key>>();
  assertSame<Tags3, Sorted<TagList<Tag2, Tag1, Tag3>, Key>>();
  assertSame<Tags3, Sorted<TagList<Tag3, Tag1, Tag2>, Key>>();
  assertSame<Tags3, Sorted<TagList<Tag1, Tag3, Tag2>, Key>>();
}

}  // namespace bits
