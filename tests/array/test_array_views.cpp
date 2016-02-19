//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "../inc_gtest.hpp"
#include "../dynd_assertions.hpp"

#include <dynd/array.hpp>
#include <dynd/array_range.hpp>
#include <dynd/func/random.hpp>
#include <dynd/types/fixed_bytes_type.hpp>

using namespace std;
using namespace dynd;

/*
TEST(ArrayViews, OneDimensionalRawMemory)
{
  nd::array a, b;
  signed char c_values[8];
  uint64_t u8_value;

  // Make an 8 byte aligned array of 80 chars
  a = nd::empty(ndt::make_type<uint64_t[10]>());
  a = a.view_scalars(ndt::make_type<char>());

  // Initialize the char values from a uint64_t,
  // to avoid having to know the endianness
  u8_value = 0x102030405060708ULL;
  memcpy(c_values, &u8_value, 8);
  a(irange() < 8).vals() = c_values;
  b = a.view_scalars<uint64_t>();
  EXPECT_EQ(ndt::make_fixed_dim(10, ndt::make_type<uint64_t>()), b.get_type());
  EXPECT_EQ(1u, b.get_shape().size());
  EXPECT_EQ(10, b.get_shape()[0]);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(u8_value, b(0).as<uint64_t>());
  b(0).vals() = 0x0505050505050505ULL;
  EXPECT_EQ(5, a(0).as<char>());

  // The system should automatically apply unaligned<>
  // where necessary
  a(1 <= irange() < 9).vals() = c_values;
  b = a(1 <= irange() < 73).view_scalars<uint64_t>();
  EXPECT_EQ(ndt::make_fixed_dim(9, ndt::view_type::make(ndt::make_type<uint64_t>(),
                                                  ndt::make_fixed_bytes(8, 1))),
            b.get_type());
  EXPECT_EQ(1u, b.get_shape().size());
  EXPECT_EQ(9, b.get_shape()[0]);
  EXPECT_EQ(a.cdata() + 1, b.cdata());
  EXPECT_EQ(u8_value, b(0).as<uint64_t>());
}

TEST(ArrayViews, MultiDimensionalRawMemory)
{
  nd::array a, b;
  uint32_t values[2][3] = {{1, 2, 3}, {0xffffffff, 0x80000000, 0}};

  a = values;

  // Should throw if the view type is the wrong size
  EXPECT_THROW(b = a.view_scalars<int16_t>(), dynd::type_error);

  b = a.view_scalars<int32_t>();
  EXPECT_EQ(ndt::make_fixed_dim(
                2, ndt::make_fixed_dim(3, ndt::make_type<int32_t>())),
            b.get_type());
  EXPECT_EQ(2u, b.get_shape().size());
  EXPECT_EQ(2, b.get_shape()[0]);
  EXPECT_EQ(3, b.get_shape()[1]);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(1, b(0, 0).as<int32_t>());
  EXPECT_EQ(2, b(0, 1).as<int32_t>());
  EXPECT_EQ(3, b(0, 2).as<int32_t>());
  EXPECT_EQ(-1, b(1, 0).as<int32_t>());
  EXPECT_EQ(std::numeric_limits<int32_t>::min(), b(1, 1).as<int32_t>());
  EXPECT_EQ(0, b(1, 2).as<int32_t>());
}
*/

TEST(ArrayViews, OneDimPermute)
{
  int vals0[3] = {0, 1, 2};

  nd::array a = nd::empty(ndt::make_type<int[3]>());
  a.vals() = vals0;

  intptr_t ndim = 1;
  intptr_t axes[1] = {0};
  nd::array b = a.permute(ndim, axes);
  EXPECT_EQ(a.cdata(), b.cdata());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(a(i).as<int>(), b(i).as<int>());
  }

  a = nd::empty(ndt::type("3 * int"));
  a.vals() = vals0;

  b = a.permute(ndim, axes);
  EXPECT_EQ(a.cdata(), b.cdata());
  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(a(i).as<int>(), b(i).as<int>());
  }
}

TEST(ArrayViews, TwoDimPermute)
{
  int vals0[3][3] = {{0, 1, 2}, {3, 4, -4}, {-3, -2, -1}};

  nd::array a = nd::empty(ndt::make_type<int[3][3]>());
  a.vals() = vals0;

  intptr_t ndim = 2;
  intptr_t axes[2] = {1, 0};
  nd::array b = a.permute(ndim, axes);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_ndim(), b.get_ndim());
  EXPECT_EQ(3, b.get_shape()[0]);
  EXPECT_EQ(3, b.get_shape()[1]);
  EXPECT_EQ(ndt::type("3 * 3 * int"), b.get_type());
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      EXPECT_EQ(a(i, j).as<int>(), b(j, i).as<int>());
    }
  }

  a = nd::empty(3, ndt::type("3 * int"));
  a.vals() = vals0;

  b = a.permute(ndim, axes);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_ndim(), b.get_ndim());
  EXPECT_EQ(a.get_shape()[0], b.get_shape()[1]);
  EXPECT_EQ(a.get_shape()[1], b.get_shape()[0]);
  EXPECT_EQ(ndt::type("3 * 3 * int"), b.get_type());
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      EXPECT_EQ(a(i, j).as<int>(), b(j, i).as<int>());
    }
  }

  a = nd::empty(ndt::type("3 * 3 * int"));
  a.vals() = vals0;

  b = a.permute(ndim, axes);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_ndim(), b.get_ndim());
  EXPECT_EQ(a.get_shape()[0], b.get_shape()[1]);
  EXPECT_EQ(a.get_shape()[1], b.get_shape()[0]);
  EXPECT_EQ(ndt::type("3 * 3 * int"), b.get_type());
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      EXPECT_EQ(a(i, j).as<int>(), b(j, i).as<int>());
    }
  }

  int vals1[4][3] = {{0, 1, 2}, {3, 4, -4}, {-3, -2, -1}, {-5, 0, 2}};

  a = nd::empty(ndt::make_type<int[4][3]>());
  a.vals() = vals1;

  b = a.permute(ndim, axes);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_ndim(), b.get_ndim());
  EXPECT_EQ(a.get_shape()[0], b.get_shape()[1]);
  EXPECT_EQ(a.get_shape()[1], b.get_shape()[0]);
  EXPECT_EQ(ndt::type("3 * 4 * int"), b.get_type());
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 3; ++j) {
      EXPECT_EQ(a(i, j).as<int>(), b(j, i).as<int>());
    }
  }

  a = nd::empty(ndt::type("4 * 3 * int"));
  a.vals() = vals1;

  b = a.permute(ndim, axes);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_ndim(), b.get_ndim());
  EXPECT_EQ(a.get_shape()[0], b.get_shape()[1]);
  EXPECT_EQ(a.get_shape()[1], b.get_shape()[0]);
  EXPECT_EQ(ndt::type("3 * 4 * int"), b.get_type());
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 3; ++j) {
      EXPECT_EQ(a(i, j).as<int>(), b(j, i).as<int>());
    }
  }
}

TEST(ArrayViews, NDimPermute)
{
  const intptr_t ndim0 = 4;
  intptr_t shape0[ndim0] = {7, 10, 15, 23};
  nd::array a = nd::rand(ndt::make_type(ndim0, shape0, "float64"));

  intptr_t axes0[ndim0] = {2, 3, 1, 0};
  nd::array b = a.permute(ndim0, axes0);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_ndim(), b.get_ndim());
  EXPECT_EQ(a.get_shape()[0], b.get_shape()[3]);
  EXPECT_EQ(a.get_shape()[1], b.get_shape()[2]);
  EXPECT_EQ(a.get_shape()[2], b.get_shape()[0]);
  EXPECT_EQ(a.get_shape()[3], b.get_shape()[1]);
  EXPECT_EQ(ndt::type("15 * 23 * 10 * 7 * float64"), b.get_type());
  for (int i = 0; i < shape0[0]; ++i) {
    for (int j = 0; j < shape0[1]; ++j) {
      for (int k = 0; k < shape0[2]; ++k) {
        for (int l = 0; l < shape0[3]; ++l) {
          EXPECT_EQ(a(i, j, k, l).as<double>(), b(k, l, j, i).as<double>());
        }
      }
    }
  }

  const intptr_t ndim1 = 5;
  intptr_t shape1[ndim1] = {5, 8, 3, 4, 2};
  a = nd::rand(ndt::make_type(ndim1, shape1, "float64"));

  intptr_t axes1[ndim1] = {1, 0, 2, 3, 4};
  b = a.permute(2, axes1);
  EXPECT_EQ(a.cdata(), b.cdata());
  EXPECT_EQ(a.get_ndim(), b.get_ndim());
  EXPECT_EQ(a.get_shape()[0], b.get_shape()[1]);
  EXPECT_EQ(a.get_shape()[1], b.get_shape()[0]);
  EXPECT_EQ(a.get_shape()[2], b.get_shape()[2]);
  EXPECT_EQ(a.get_shape()[3], b.get_shape()[3]);
  EXPECT_EQ(a.get_shape()[4], b.get_shape()[4]);
  EXPECT_EQ(ndt::type("8 * 5 * 3 * 4 * 2 * float64"), b.get_type());
  for (int i = 0; i < shape1[0]; ++i) {
    for (int j = 0; j < shape1[1]; ++j) {
      for (int k = 0; k < shape1[2]; ++k) {
        for (int l = 0; l < shape1[3]; ++l) {
          for (int m = 0; m < shape1[4]; ++m) {
            EXPECT_EQ(a(i, j, k, l, m).as<double>(),
                      b(j, i, k, l, m).as<double>());
          }
        }
      }
    }
  }
}

TEST(ArrayViews, NDimPermute_BadPerms)
{
  nd::array a;
  const int ndim1 = 5;
  a = nd::empty("5 * 8 * var * 4 * 2 * float64");

  // A dimension may not be permuted across a var dimension
  intptr_t axes1[ndim1] = {1, 3, 2, 0, 4};
  EXPECT_THROW(a.permute(ndim1, axes1), invalid_argument);

  a = nd::empty("5 * 8 * var * var * 2 * float64");

  // A var dimension dimension may not change position
  intptr_t axes2[ndim1] = {0, 1, 3, 2, 4};
  EXPECT_THROW(a.permute(ndim1, axes2), invalid_argument);

  a = nd::empty("5 * 8 * 3 * 4 * 2 * float64");

  // The permutation must be valid and not be larger than ndim
  intptr_t axes3[ndim1] = {0, 1, 2, 3, 5};
  EXPECT_THROW(a.permute(ndim1, axes3), invalid_argument);
  intptr_t axes4[ndim1] = {0, 1, 2, 3, 0};
  EXPECT_THROW(a.permute(ndim1, axes4), invalid_argument);
  intptr_t axes5[ndim1 + 1] = {0, 1, 2, 3, 4, 5};
  EXPECT_THROW(a.permute(ndim1 + 1, axes5), invalid_argument);
}

#ifndef DYND_NESTED_INIT_LIST_BUG
TEST(ArrayViews, Reshape)
{
  EXPECT_ARRAY_EQ(nd::array({{0, 1}, {2, 3}}), nd::reshape(nd::range(4), {2, 2}));
  EXPECT_ARRAY_EQ(nd::range(4), nd::reshape({{0, 1}, {2, 3}}, {4}));

  EXPECT_ARRAY_EQ(nd::array({{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}}),
                nd::reshape(nd::range(10), {5, 2}));
  EXPECT_ARRAY_EQ(nd::range(10),
                nd::reshape({{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}}, {10}));

  EXPECT_ARRAY_EQ(nd::array({{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}}),
                nd::reshape(nd::range(10), {2, 5}));
  EXPECT_ARRAY_EQ(nd::range(10),
                nd::reshape({{0, 1, 2, 3, 4}, {5, 6, 7, 8, 9}}, {10}));

  EXPECT_ARRAY_EQ(nd::array({{{0, 1}, {2, 3}}, {{4, 5}, {6, 7}}}),
                nd::reshape(nd::range(8), {2, 2, 2}));
  EXPECT_ARRAY_EQ(nd::range(8),
                nd::reshape({{{0, 1}, {2, 3}}, {{4, 5}, {6, 7}}}, {8}));

  EXPECT_ARRAY_EQ(
      nd::array({{{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}},
                 {{12, 13, 14, 15}, {16, 17, 18, 19}, {20, 21, 22, 23}}}),
      nd::reshape(nd::range(24), {2, 3, 4}));
  EXPECT_ARRAY_EQ(
      nd::range(24),
      nd::reshape({{{0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}},
                   {{12, 13, 14, 15}, {16, 17, 18, 19}, {20, 21, 22, 23}}},
                  {24}));

  EXPECT_ARRAY_EQ(nd::array({{{0, 1}, {2, 3}, {4, 5}},
                           {{6, 7}, {8, 9}, {10, 11}},
                           {{12, 13}, {14, 15}, {16, 17}},
                           {{18, 19}, {20, 21}, {22, 23}}}),
                nd::reshape(nd::range(24), {4, 3, 2}));
  EXPECT_ARRAY_EQ(nd::range(24), nd::reshape({{{0, 1}, {2, 3}, {4, 5}},
                                            {{6, 7}, {8, 9}, {10, 11}},
                                            {{12, 13}, {14, 15}, {16, 17}},
                                            {{18, 19}, {20, 21}, {22, 23}}},
                                           {24}));
}
#endif // DYND_NESTED_INIT_LIST_BUG
