//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/functional.hpp>
#include <dynd/index.hpp>
#include <dynd/kernels/index_kernel.hpp>

using namespace std;
using namespace dynd;

DYND_API nd::callable nd::index::make()
{
  typedef type_id_sequence<int32_type_id, fixed_dim_type_id> type_ids;

  auto children = callable::make_all<index_kernel, type_ids>();
  nd::callable f = functional::dispatch(
      ndt::type("(Any, i: Any) -> Any"),
      [children](const ndt::type &DYND_UNUSED(dst_tp), intptr_t DYND_UNUSED(nsrc),
                 const ndt::type *src_tp) mutable -> callable & { return children[src_tp[0].get_type_id()]; });
  f->kernreq = kernel_request_call;

  return f;
}

DYND_API struct nd::index nd::index;