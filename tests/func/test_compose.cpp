//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include "inc_gtest.hpp"

#include <dynd/array.hpp>
#include <dynd/callable.hpp>
#include <dynd/types/fixed_string_type.hpp>
#include <dynd/types/date_type.hpp>
#include <dynd/func/elwise.hpp>
#include <dynd/func/take.hpp>
#include <dynd/func/compose.hpp>
#include <dynd/func/copy.hpp>
#include <dynd/callable_registry.hpp>
#include <dynd/convert.hpp>

using namespace std;
using namespace dynd;

TEST(Compose, Simple)
{
  nd::callable composed =
      nd::functional::compose(nd::copy::get(), nd::callable_registry["sin"], ndt::make_type<double>());
  nd::array a = nd::empty(ndt::make_type<double>());
  composed({"0.0"}, {{"dst", a}});
  EXPECT_EQ(0., a.as<double>());
  composed({"1.5"}, {{"dst", a}});
  EXPECT_DOUBLE_EQ(sin(1.5), a.as<double>());
  composed({3.1}, {{"dst", a}});
  EXPECT_DOUBLE_EQ(sin(3.1), a.as<double>());
}

/*
TEST(Convert, Unary)
{
  nd::callable f = nd::functional::apply([](double x) { return x; });
  nd::callable g = nd::functional::convert(ndt::type("(float32) -> float64"), f);
}
*/
