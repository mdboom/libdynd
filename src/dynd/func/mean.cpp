//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/func/mean.hpp>
#include <dynd/array.hpp>
#include <dynd/types/fixed_dim_kind_type.hpp>
#include <dynd/func/reduction.hpp>
#include <dynd/kernels/base_kernel.hpp>
#include <dynd/kernels/sum_kernel.hpp>
#include <dynd/kernels/mean_kernel.hpp>

using namespace std;
using namespace dynd;

DYND_API nd::callable nd::mean::make() { return callable::make<mean_kernel>(ndt::type(int64_id)); }

DYND_API struct nd::mean nd::mean;
