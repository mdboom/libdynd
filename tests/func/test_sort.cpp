//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "inc_gtest.hpp"

#include <dynd/sort.hpp>

#include "dynd_assertions.hpp"

using namespace std;
using namespace dynd;

TEST(Sort, 1D)
{
  nd::array a{2.5, 1.25, 0.0};
  nd::sort(a);
  EXPECT_ARRAY_EQ((nd::array{0.0, 1.25, 2.5}), a);

  a = {19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  nd::sort(a);
  EXPECT_ARRAY_EQ((nd::array{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19}), a);
}

/*
TEST(Unique, 1D)
{
  nd::array a{0, 0, 1, 2, 2, 3};
  nd::unique(a);
  EXPECT_ARRAY_EQ((nd::array{0, 1, 2, 3}), a);
}
*/
