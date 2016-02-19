//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#include <benchmark/benchmark.h>

#include <dynd/func/arithmetic.hpp>
#include <dynd/func/random.hpp>

using namespace std;
using namespace dynd;

static const int size = 10000; // 256 * 256

static void BM_Func_Arithmetic_Add(benchmark::State &state)
{
  nd::array a = nd::random::uniform({}, {{"dst_tp", ndt::make_fixed_dim(size, ndt::make_type<float>())}});
  nd::array b = nd::random::uniform({}, {{"dst_tp", ndt::make_fixed_dim(size, ndt::make_type<float>())}});
  nd::array c = nd::empty(ndt::make_fixed_dim(size, ndt::make_type<float>()));
  while (state.KeepRunning()) {
    nd::add(a, b);
  }
}

BENCHMARK(BM_Func_Arithmetic_Add);

static void BM_Func_Arithmetic_Dispatch_time(benchmark::State &state)
{
  nd::array a = 5;
  nd::array b = (short)6;
  while (state.KeepRunning()) {
    nd::add(a, a);
    nd::add(b, b);
    nd::add(a, b);
    nd::add(b, a);
  }
}

BENCHMARK(BM_Func_Arithmetic_Dispatch_time);

static void BM_Func_Arithmetic_Dispatch_time_2(benchmark::State &state)
{
  nd::array a = (char)2;
  nd::array b = (dynd::complex128)1.;
  while (state.KeepRunning()) {
    nd::add(a, a);
    nd::add(b, b);
  }
}

BENCHMARK(BM_Func_Arithmetic_Dispatch_time_2);

static void BM_Func_Arithmetic_Dispatch_time_3(benchmark::State &state)
{
  nd::array a = (char)2;
  while (state.KeepRunning()) {
    nd::add(a, a);
  }
}

BENCHMARK(BM_Func_Arithmetic_Dispatch_time_3);

static void BM_Func_Arithmetic_Dispatch_time_4(benchmark::State &state)
{
  nd::array b = (dynd::complex128)1.;
  while (state.KeepRunning()) {
    nd::add(b, b);
  }
}

BENCHMARK(BM_Func_Arithmetic_Dispatch_time_4);
