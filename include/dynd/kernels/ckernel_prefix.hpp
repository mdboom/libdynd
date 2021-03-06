//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <dynd/config.hpp>
#include <dynd/diagnostics.hpp>

namespace dynd {
namespace nd {

  class array;

} // namespace dynd::nd

namespace eval {

  struct DYND_API eval_context;
  extern DYND_API eval_context default_eval_context;

} // namespace dynd::eval

namespace ndt {

  class type;

} // namespace dynd::ndt

struct ckernel_prefix;

typedef void (*kernel_call_t)(ckernel_prefix *self, nd::array *dst, nd::array *const *src);
typedef void (*kernel_single_t)(ckernel_prefix *self, char *dst, char *const *src);
typedef void (*kernel_strided_t)(ckernel_prefix *self, char *dst, intptr_t dst_stride, char *const *src,
                                 const intptr_t *src_stride, size_t count);

struct kernel_targets_t {
  void *single;
  void *contiguous;
  void *strided;
};

/**
 * Definition for kernel request parameters.
 */
enum {
  /** Kernel function in host memory */
  kernel_request_host = 0x00000000,

  kernel_request_call = 0x00000000,
  kernel_request_single = 0x00000001,
  kernel_request_strided = 0x00000003,

  kernel_request_data_only = 0x00000001
};
typedef uint32_t kernel_request_t;

/**
 * This is the struct which begins the memory layout
 * of all ckernels. First comes the function pointer,
 * which has a context-specific prototype, such as
 * `kernel_single_t`, and then comes the
 * destructor.
 *
 * The ckernel is defined in terms of a C ABI definition,
 * and must satisfy alignments, movability, and
 * concurrency requirements to be valid. See the
 * document
 */
struct DYND_API ckernel_prefix {
  typedef void (*destructor_fn_t)(ckernel_prefix *);

  void (*destructor)(ckernel_prefix *self);
  void *function;

  /**
   * Aligns an offset as required by ckernels.
   */
  DYND_CUDA_HOST_DEVICE static size_t align_offset(size_t offset) { return (offset + size_t(7)) & ~size_t(7); }

  /**
   * Call to get the kernel function pointer, whose type
   * must be known by the context.
   *
   *      kdp->get_function<kernel_single_t>()
   */
  template <typename T>
  DYND_CUDA_HOST_DEVICE T get_function() const
  {
    return reinterpret_cast<T>(function);
  }

  /**
   * Calls the destructor of the ckernel if it is
   * non-NULL.
   */
  DYND_CUDA_HOST_DEVICE void destroy()
  {
    if (destructor != NULL) {
      destructor(this);
    }
  }

  DYND_CUDA_HOST_DEVICE void single(char *dst, char *const *src)
  {
    (*reinterpret_cast<kernel_single_t>(function))(this, dst, src);
  }

  DYND_CUDA_HOST_DEVICE void strided(char *dst, intptr_t dst_stride, char *const *src, const intptr_t *src_stride,
                                     size_t count)
  {
    (*reinterpret_cast<kernel_strided_t>(function))(this, dst, dst_stride, src, src_stride, count);
  }

  /**
   * Returns the pointer to a child ckernel at the provided
   * offset.
   */
  DYND_CUDA_HOST_DEVICE ckernel_prefix *get_child(intptr_t offset)
  {
    return reinterpret_cast<ckernel_prefix *>(reinterpret_cast<char *>(this) + ckernel_prefix::align_offset(offset));
  }

  static ckernel_prefix *init(ckernel_prefix *self, kernel_request_t DYND_UNUSED(kernreq), void *func)
  {
    self->function = func;
    self->destructor = NULL;
    return self;
  }

  template <typename CKBT>
  static ckernel_prefix *make(CKBT *ckb, kernel_request_t kernreq, intptr_t &inout_ckb_offset, void *func);

  static char *data_init(char *DYND_UNUSED(static_data), const ndt::type &DYND_UNUSED(dst_tp),
                         intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp), intptr_t DYND_UNUSED(nkwd),
                         const nd::array *DYND_UNUSED(kwds),
                         const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))
  {
    return NULL;
  }

  static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb, intptr_t ckb_offset,
                              const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                              intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp),
                              const char *const *DYND_UNUSED(src_arrmeta), kernel_request_t kernreq,
                              intptr_t DYND_UNUSED(nkwds), const nd::array *DYND_UNUSED(kwds),
                              const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars));
};

} // namespace dynd
