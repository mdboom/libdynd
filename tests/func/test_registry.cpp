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
#include <dynd/callable_registry.hpp>

using namespace std;
using namespace dynd;

/*
TEST(CallableRegistry, Dispatch)
{
  nd::callable af;
  af = nd::callable_registry["sin");
  // These are exact overloads of ``sin``
  EXPECT_DOUBLE_EQ(sin(1.0), af(1.0).as<double>());
  EXPECT_FLOAT_EQ(sin(2.0f), af(2.0f).as<float>());
  // Implicit int -> double conversion
  EXPECT_DOUBLE_EQ(sin(3.0), af(3).as<double>());
  EXPECT_DOUBLE_EQ(sin(4.0), af(4u).as<double>());
  // Bool doesn't implicitly convert to float
  EXPECT_THROW(af(true), type_error);
}
*/

TEST(CallableRegistry, Arithmetic)
{
  // Simple sanity checks
  nd::callable af;
  af = nd::callable_registry["add"];
  EXPECT_EQ(ndt::type("int32"), af((int8_t)3, (int8_t)4).get_type());
  EXPECT_EQ(8, af(3, 5).as<int>());
  EXPECT_EQ(ndt::type("float32"), af(3.5f, 5.25f).get_type());
  EXPECT_EQ(8.75, af(3.5f, 5.25f).as<float>());
  af = nd::callable_registry["subtract"];
  EXPECT_EQ(ndt::type("float64"), af(3.5, 4).get_type());
  EXPECT_EQ(-0.5, af(3.5, 4).as<double>());
  af = nd::callable_registry["multiply"];
  EXPECT_EQ(ndt::type("float32"), af(3.5f, (int8_t)4).get_type());
  EXPECT_EQ(14, af(3.5f, (int8_t)4).as<float>());
  af = nd::callable_registry["divide"];
  EXPECT_EQ(ndt::type("float64"), af(12.0, (int8_t)4).get_type());
  EXPECT_EQ(3, af(12.0, (int8_t)4).as<double>());
}

TEST(CallableRegistry, Trig)
{
  // Simple sanity checks
  nd::callable af;
  af = nd::callable_registry["sin"];
  EXPECT_FLOAT_EQ(sinf(2.0f), af(2.0f).as<float>());
  EXPECT_DOUBLE_EQ(sin(1.0), af(1.0).as<double>());
  af = nd::callable_registry["cos"];
  EXPECT_FLOAT_EQ(cosf(1.f), af(1.f).as<float>());
  EXPECT_DOUBLE_EQ(cos(1.0), af(1.0).as<double>());
  af = nd::callable_registry["tan"];
  EXPECT_FLOAT_EQ(tanf(1.f), af(1.f).as<float>());
  EXPECT_DOUBLE_EQ(tan(1.0), af(1.0).as<double>());
  af = nd::callable_registry["exp"];
  EXPECT_FLOAT_EQ(expf(1.f), af(1.f).as<float>());
  EXPECT_DOUBLE_EQ(exp(1.0), af(1.0).as<double>());
  af = nd::callable_registry["arcsin"];
  EXPECT_FLOAT_EQ(asinf(0.4f), af(0.4f).as<float>());
  EXPECT_DOUBLE_EQ(asin(1.0), af(1.0).as<double>());
  af = nd::callable_registry["arccos"];
  EXPECT_FLOAT_EQ(acosf(1.f), af(1.f).as<float>());
  EXPECT_DOUBLE_EQ(acos(1.0), af(1.0).as<double>());
  af = nd::callable_registry["arctan"];
  EXPECT_FLOAT_EQ(atanf(1.f), af(1.f).as<float>());
  EXPECT_DOUBLE_EQ(atan(1.0), af(1.0).as<double>());
  af = nd::callable_registry["arctan2"];
  EXPECT_FLOAT_EQ(atan2f(1.f, 2.f), af(1.f, 2.f).as<float>());
  EXPECT_DOUBLE_EQ(atan2(1.0, 2.0), af(1.0, 2.0).as<double>());
  af = nd::callable_registry["hypot"];
  EXPECT_FLOAT_EQ(5, af(3.f, 4.f).as<float>());
  EXPECT_DOUBLE_EQ(hypot(3.0, 4.5), af(3.0, 4.5).as<double>());
  af = nd::callable_registry["sinh"];
  EXPECT_FLOAT_EQ(sinhf(2.0f), af(2.0f).as<float>());
  EXPECT_DOUBLE_EQ(sinh(1.0), af(1.0).as<double>());
  af = nd::callable_registry["cosh"];
  EXPECT_FLOAT_EQ(coshf(1.f), af(1.f).as<float>());
  EXPECT_DOUBLE_EQ(cosh(1.0), af(1.0).as<double>());
  af = nd::callable_registry["tanh"];
  EXPECT_FLOAT_EQ(tanhf(1.f), af(1.f).as<float>());
  EXPECT_DOUBLE_EQ(tanh(1.0), af(1.0).as<double>());
  af = nd::callable_registry["power"];
  EXPECT_FLOAT_EQ(powf(1.5f, 2.25f), af(1.5f, 2.25f).as<float>());
  EXPECT_DOUBLE_EQ(pow(1.5, 2.25), af(1.5, 2.25).as<double>());
}
