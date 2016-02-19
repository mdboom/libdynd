//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <memory>

#include <dynd/callables/base_callable.hpp>

using namespace std;
using namespace dynd;

nd::array nd::base_callable::call(ndt::type &dst_tp, intptr_t nsrc, const ndt::type *src_tp,
                                  const char *const *src_arrmeta, char *const *src_data, intptr_t nkwd,
                                  const array *kwds, const std::map<std::string, ndt::type> &tp_vars)
{
  // Allocate, then initialize, the data
  char *data = data_init(static_data(), dst_tp, nsrc, src_tp, nkwd, kwds, tp_vars);

  // Resolve the destination type
  if (dst_tp.is_symbolic()) {
    if (resolve_dst_type == NULL) {
      throw std::runtime_error("dst_tp is symbolic, but resolve_dst_type is NULL");
    }

    resolve_dst_type(static_data(), data, dst_tp, nsrc, src_tp, nkwd, kwds, tp_vars);
  }

  // Allocate the destination array
  array dst = empty(dst_tp);

  // Generate and evaluate the ckernel
  kernel_builder ckb;
  instantiate(static_data(), data, &ckb, dst_tp, dst.get()->metadata(), nsrc, src_tp, src_arrmeta,
              kernel_request_single, nkwd, kwds, tp_vars);
  kernel_single_t fn = ckb.get()->get_function<kernel_single_t>();
  fn(ckb.get(), dst.data(), src_data);

  return dst;
}

nd::array nd::base_callable::call(ndt::type &dst_tp, intptr_t nsrc, const ndt::type *src_tp,
                                  const char *const *src_arrmeta, const array *src_data, intptr_t nkwd,
                                  const array *kwds, const std::map<std::string, ndt::type> &tp_vars)
{
  // Allocate, then initialize, the data
  char *data = data_init(static_data(), dst_tp, nsrc, src_tp, nkwd, kwds, tp_vars);

  // Resolve the destination type
  if (dst_tp.is_symbolic()) {
    if (resolve_dst_type == NULL) {
      throw std::runtime_error("dst_tp is symbolic, but resolve_dst_type is NULL");
    }

    resolve_dst_type(static_data(), data, dst_tp, nsrc, src_tp, nkwd, kwds, tp_vars);
  }

  // Allocate the destination array
  array dst = empty(dst_tp);

  // Generate and evaluate the ckernel
  kernel_builder ckb;
  instantiate(static_data(), data, &ckb, dst_tp, dst.get()->metadata(), nsrc, src_tp, src_arrmeta, kernel_request_call,
              nkwd, kwds, tp_vars);
  kernel_call_t fn = ckb.get()->get_function<kernel_call_t>();
  fn(ckb.get(), &dst, src_data);

  return dst;
}

void nd::base_callable::call(const ndt::type &dst_tp, const char *dst_arrmeta, char *dst_data, intptr_t nsrc,
                             const ndt::type *src_tp, const char *const *src_arrmeta, char *const *src_data,
                             intptr_t nkwd, const array *kwds, const std::map<std::string, ndt::type> &tp_vars)
{
  char *data = data_init(static_data(), dst_tp, nsrc, src_tp, nkwd, kwds, tp_vars);

  // Generate and evaluate the ckernel
  kernel_builder ckb;
  instantiate(static_data(), data, &ckb, dst_tp, dst_arrmeta, nsrc, src_tp, src_arrmeta, kernel_request_single, nkwd,
              kwds, tp_vars);
  kernel_single_t fn = ckb.get()->get_function<kernel_single_t>();
  fn(ckb.get(), dst_data, src_data);
}

void nd::base_callable::call(const ndt::type &dst_tp, const char *dst_arrmeta, array *dst, intptr_t nsrc,
                             const ndt::type *src_tp, const char *const *src_arrmeta, const array *src, intptr_t nkwd,
                             const array *kwds, const std::map<std::string, ndt::type> &tp_vars)
{
  char *data = data_init(static_data(), dst_tp, nsrc, src_tp, nkwd, kwds, tp_vars);

  // Generate and evaluate the ckernel
  kernel_builder ckb;
  instantiate(static_data(), data, &ckb, dst_tp, dst_arrmeta, nsrc, src_tp, src_arrmeta, kernel_request_call, nkwd,
              kwds, tp_vars);
  kernel_call_t fn = ckb.get()->get_function<kernel_call_t>();
  fn(ckb.get(), dst, src);
}
