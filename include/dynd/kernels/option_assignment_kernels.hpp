//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/callable.hpp>

namespace dynd {
namespace kernels {

  /**
   * Makes a ckernel for assignments containing an option type.
   */
  DYND_API size_t make_option_assignment_kernel(void *ckb, intptr_t ckb_offset, const ndt::type &dst_tp,
                                                const char *dst_arrmeta, const ndt::type &src_tp,
                                                const char *src_arrmeta, kernel_request_t kernreq);

} // namespace dynd::kernels
} // namespace dynd
