//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/callable.hpp>
#include <dynd/string_search.hpp>

namespace dynd {

/*
  Concatenates `nop` strings in the `s` array, storing the result in
  `d`
*/
template <class StringType>
void string_concat(size_t nop, StringType &d, const StringType *const *s)
{
  // Get the size of the concatenated string
  size_t size = 0;
  for (size_t i = 0; i != nop; ++i) {
    size += (s[i]->size());
  }

  // Allocate the output
  d.resize(size);
  // Copy the string data
  char *dst = d.begin();
  for (size_t i = 0; i != nop; ++i) {
    size_t op_size = (s[i]->size());
    DYND_MEMCPY(dst, s[i]->begin(), op_size);
    dst += op_size;
  }
}

/*
  Returns the number of times needle appears in haystack.
*/
template <class StringType>
intptr_t string_count(const StringType &haystack, const StringType &needle)
{
  detail::string_counter<StringType> f;

  f(haystack, needle);

  return f.finish();
}

/*
  Returns byte index of the first occurrence of needle in haystack.
  Returns -1 if not found.
*/
template <class StringType>
intptr_t string_find(const StringType &haystack, const StringType &needle)
{
  detail::string_finder<StringType> f;

  f(haystack, needle);

  return f.finish();
}

/*
  In string `src`, replace all non-overlapping appearances of
  `old_str` with `new_str`, storing the result in `dst`.
*/
template <class StringType>
void string_replace(StringType &dst, const StringType &src, const StringType &old_str, const StringType &new_str)
{

  if (old_str.size() == 0 || old_str.size() > src.size()) {
    /* Just copy -- there's nothing to replace */
    dst = src;
  }
  else if (old_str.size() == new_str.size()) {
    /* Special case when old_str and new_str are same length,
       we copy src to dst and the replace in-place. */
    dst = src;

    if (old_str.size() == 1) {
      /* Special case when old_str and new_str are both 1 character */
      char old_chr = old_str.begin()[0];
      char new_chr = new_str.begin()[0];
      for (auto p = dst.begin(); p != dst.end(); ++p) {
        if (*p == old_chr) {
          *p = new_chr;
        }
      }
    }
    else {
      detail::string_inplace_replacer<StringType> replacer(dst, new_str);
      replacer(src, old_str);
    }
  }
  else {
    /* Most general case, where old_str and new_str are different
       lengths.  Count matches to determine resulting string length,
       then interleave to make the result. */
    intptr_t count = string_count(src, old_str);
    size_t delta = ((intptr_t)new_str.size() - (intptr_t)old_str.size()) * count;

    dst.resize((intptr_t)src.size() + delta);

    detail::string_copy_replacer<StringType> replacer(dst, src, old_str, new_str);
    replacer(src, old_str);
    replacer.finish();
  }
}

namespace nd {

  extern DYND_API struct DYND_API string_concatenation : declfunc<string_concatenation> {
    static callable make();
  } string_concatenation;

  extern DYND_API struct DYND_API string_count : declfunc<string_count> {
    static callable make();
  } string_count;

  extern DYND_API struct DYND_API string_find : declfunc<string_find> {
    static callable make();
  } string_find;

  extern DYND_API struct DYND_API string_replace : declfunc<string_replace> {
    static callable make();
  } string_replace;

  extern DYND_API struct DYND_API string_split : declfunc<string_split> {
    static callable make();
  } string_split;

} // namespace dynd::nd
} // namespace dynd
