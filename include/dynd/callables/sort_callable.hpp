//
// Copyright (C) 2011-16 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/comparison.hpp>
#include <dynd/callables/base_callable.hpp>
#include <dynd/kernels/sort_kernel.hpp>

namespace dynd {
namespace nd {

  class sort_callable : public base_callable {
    struct node_type : call_node {
      size_t data_size;

      node_type(base_callable *callee, size_t data_size) : call_node(callee), data_size(data_size) {}
    };

  public:
    sort_callable()
        : base_callable(ndt::callable_type::make(ndt::make_type<void>(), {ndt::type("Fixed * Scalar")}),
                        sizeof(node_type)) {}

    ndt::type resolve(base_callable *DYND_UNUSED(caller), char *DYND_UNUSED(data), call_graph &cg,
                      const ndt::type &dst_tp, size_t DYND_UNUSED(nsrc), const ndt::type *src_tp,
                      size_t DYND_UNUSED(nkwd), const array *DYND_UNUSED(kwds),
                      const std::map<std::string, ndt::type> &tp_vars) {
      const ndt::type &src0_element_tp = src_tp[0].extended<ndt::fixed_dim_type>()->get_element_type();
      cg.emplace_back<node_type>(this, src0_element_tp.get_data_size());

      const ndt::type child_src_tp[2] = {src0_element_tp, src0_element_tp};
      less->resolve(this, nullptr, cg, ndt::make_type<bool1>(), 2, child_src_tp, 0, nullptr, tp_vars);

      return dst_tp;
    }

    void instantiate(call_node *&node, char *DYND_UNUSED(data), kernel_builder *ckb,
                     const ndt::type &DYND_UNUSED(dst_tp), const char *DYND_UNUSED(dst_arrmeta),
                     intptr_t DYND_UNUSED(nsrc), const ndt::type *DYND_UNUSED(src_tp), const char *const *src_arrmeta,
                     kernel_request_t kernreq, intptr_t nkwd, const nd::array *kwds,
                     const std::map<std::string, ndt::type> &tp_vars) {
      ckb->emplace_back<sort_kernel>(kernreq,
                                     reinterpret_cast<const fixed_dim_type_arrmeta *>(src_arrmeta[0])->dim_size,
                                     reinterpret_cast<const fixed_dim_type_arrmeta *>(src_arrmeta[0])->stride,
                                     reinterpret_cast<node_type *>(node)->data_size);
      node = next(node);

      node->callee->instantiate(node, nullptr, ckb, ndt::type(), nullptr, 2, nullptr, nullptr, kernel_request_single,
                                nkwd, kwds, tp_vars);
    }
  };

} // namespace dynd::nd
} // namespace dynd
