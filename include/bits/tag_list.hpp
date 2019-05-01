#pragma once

#include <type_traits>

namespace bits {

template <typename... Tags>
struct TagList;

namespace detail {

// ----------------------------------------------------------------------------
// ------------------------------------- If -----------------------------------
// ----------------------------------------------------------------------------

template <bool Condition, typename Then, typename Else>
using If = typename std::conditional<Condition, Then, Else>::type;

// ----------------------------------------------------------------------------
// ----------------------------------- FoldL ----------------------------------
// ----------------------------------------------------------------------------

template <typename Acc, template <typename, typename> class F, typename... Tags>
struct FoldL;

template <typename Acc, template <typename, typename> class F>
struct FoldL<Acc, F> {
  using type = Acc;
};

template <typename Acc, template <typename, typename> class F, typename HeadTag,
          typename... Tags>
struct FoldL<Acc, F, HeadTag, Tags...> {
  using type = typename FoldL<typename F<Acc, HeadTag>::type, F, Tags...>::type;
};

// ----------------------------------------------------------------------------
// ---------------------------------- HasTag ----------------------------------
// ----------------------------------------------------------------------------

template <typename LookupTag, typename... Tags>
struct HasTag {
  template <typename Acc, typename Tag>
  struct F {
    using type =
        If<Acc::value, Acc, typename std::is_same<Tag, LookupTag>::type>;
  };

  using type = typename FoldL<std::false_type, F, Tags...>::type;

  static constexpr auto value = type::value;
};

// ----------------------------------------------------------------------------
// ---------------------------------- Prepend ---------------------------------
// ----------------------------------------------------------------------------

template <typename Tag, typename... Tags>
struct PrependVargs {
  using type = ::bits::TagList<Tag, Tags...>;
};

template <typename TagList, typename Tag>
struct PrependTagList {
  using type = typename TagList::template Apply<PrependVargs, Tag>::type;
};

// ----------------------------------------------------------------------------
// ---------------------------------- Append ----------------------------------
// ----------------------------------------------------------------------------

template <typename Tag, typename... Tags>
struct AppendVargs {
  using type = ::bits::TagList<Tags..., Tag>;
};

template <typename TagList, typename Tag>
struct AppendTagList {
  using type = typename TagList::template Apply<AppendVargs, Tag>::type;
};

// ----------------------------------------------------------------------------
// ---------------------------------- Reverse ---------------------------------
// ----------------------------------------------------------------------------

template <typename... Tags>
struct Reversed {
  using type = typename FoldL<TagList<>, PrependTagList, Tags...>::type;
};

// ----------------------------------------------------------------------------
// ------------------------------- Concatenated -------------------------------
// ----------------------------------------------------------------------------

template <typename LhsTagList, typename... Tags>
struct ConcatenatedTags {
  using type = typename FoldL<LhsTagList, AppendTagList, Tags...>::type;
};

template <typename LhsTagList, typename RhsTagList>
struct ConcatenatedTagList {
  using type =
      typename RhsTagList::template Apply<ConcatenatedTags, LhsTagList>::type;
};

// ----------------------------------------------------------------------------
// --------------------------------- Filtered ---------------------------------
// ----------------------------------------------------------------------------

template <typename TagList, template <typename> class F>
struct FilteredTagList {
  template <typename FilteredTagList, typename Tag>
  struct F1 {
    using type =
        If<F<Tag>::value, typename AppendTagList<FilteredTagList, Tag>::type,
           FilteredTagList>;
  };

  template <typename... Tags>
  struct F2 {
    using type = typename FoldL<::bits::TagList<>, F1, Tags...>::type;
  };

  using type = typename TagList::template Apply<F2>::type;
};

// ----------------------------------------------------------------------------
// ------------------------------- Partitioned- -------------------------------
// ----------------------------------------------------------------------------

template <typename TagList, template <typename> class F>
struct Partitioned {
  template <typename Tag>
  struct FInv {
    static constexpr auto value = !F<Tag>::value;
  };

  using lhs = typename FilteredTagList<TagList, F>::type;
  using rhs = typename FilteredTagList<TagList, FInv>::type;
};

template <typename TagList, template <typename, typename> class F>
struct SortedTagList {
  template <typename... Tags>
  struct SortedVargs {
    using type = ::bits::TagList<>;
  };

  template <typename HeadTag, typename... Tags>
  struct SortedVargs<HeadTag, Tags...> {
    template <typename Tag>
    struct F1 {
      static constexpr auto value = F<Tag, HeadTag>::value;
    };

    using lhs = typename Partitioned<::bits::TagList<Tags...>, F1>::lhs;
    using rhs = typename Partitioned<::bits::TagList<Tags...>, F1>::rhs;
    using type = typename ConcatenatedTagList<
        typename AppendTagList<typename SortedTagList<lhs, F>::type,
                               HeadTag>::type,
        typename SortedTagList<rhs, F>::type>::type;
  };

  using type = typename TagList::template Apply<SortedVargs>::type;
};

}  // namespace detail

// A list of tags/types which can be manipulated (in a pure functional style at
// compile time!) using the functions below.
template <typename... Tags>
struct TagList {
  // Implementation detail, ignore. Must be "... class M>" not "... typename
  // M>"! Why?
  template <template <typename...> class M, typename... Args>
  using Apply = M<Args..., Tags...>;
};

// Returns true if the LookupTag is present in the TagList and false otherwise.
template <typename TagList, typename LookupTag>
constexpr bool hasTag() {
  return TagList::template Apply<detail::HasTag, LookupTag>::value;
}

// Generates a reversed TagList.
template <typename TagList>
using Reversed = typename TagList::template Apply<detail::Reversed>::type;

// Generates a concatenation of LhsTagList and RhsTagList.
template <typename LhsTagList, typename RhsTagList>
using Concatenated =
    typename detail::ConcatenatedTagList<LhsTagList, RhsTagList>::type;

// Generates a TagList containing only tag T's for which F<T>::value is true.
template <typename TagList, template <typename> class F>
using Filtered = typename detail::FilteredTagList<TagList, F>::type;

// Generates a partitioning of TagList into lhs and rhs. The lhs has all tag T's
// for which F<T>::value is true and rhs all other tags.
template <typename TagList, template <typename> class F>
struct Partitioned {
  using lhs = typename detail::Partitioned<TagList, F>::lhs;
  using rhs = typename detail::Partitioned<TagList, F>::rhs;
};

// Generates a sorted (from smallest to largest) TagList based on the "less
// than" comparator F<Lhs, Rhs>::value for pairs of type Lhs and Rhs.
template <typename TagList, template <typename, typename> class F>
using Sorted = typename detail::SortedTagList<TagList, F>::type;

}  // namespace bits
