//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/functional.hpp>
#include <dynd/option.hpp>
#include <dynd/kernels/assign_na_kernel.hpp>
#include <dynd/kernels/is_missing_kernel.hpp>

using namespace std;
using namespace dynd;

DYND_API nd::callable nd::assign_na::make()
{
  typedef type_id_sequence<bool_type_id, int8_type_id, int16_type_id, int32_type_id, int64_type_id, int128_type_id,
                           float32_type_id, float64_type_id, complex_float32_type_id, complex_float64_type_id,
                           void_type_id, string_type_id, fixed_dim_type_id, date_type_id, time_type_id,
                           datetime_type_id> type_ids;

  std::map<type_id_t, callable> children = callable::make_all<assign_na_kernel, type_ids>();
  children[uint32_type_id] = callable::make<assign_na_kernel<uint32_type_id>>();
  std::array<callable, 2> dim_children;

  auto t = ndt::type("() -> ?Any");
  callable self = functional::call<assign_na>(t);

  for (auto tp_id : {fixed_dim_type_id, var_dim_type_id}) {
    dim_children[tp_id - fixed_dim_type_id] = functional::elwise(self);
  }

  return functional::dispatch(t, [children, dim_children](const ndt::type &dst_tp, intptr_t DYND_UNUSED(nsrc),
                                                          const ndt::type *DYND_UNUSED(src_tp)) mutable -> callable & {
    callable *child = nullptr;
    if (dst_tp.get_kind() == option_kind) {
      child = &children[dst_tp.extended<ndt::option_type>()->get_value_type().get_type_id()];
    }
    else
      child = &dim_children[dst_tp.get_type_id() - fixed_dim_type_id];

    if (child->is_null()) {
      throw std::runtime_error("no child found");
    }

    return *child;
  });
}

DYND_API struct nd::assign_na nd::assign_na;

DYND_API nd::callable nd::is_missing::make()
{
  typedef type_id_sequence<bool_type_id, int8_type_id, int16_type_id, int32_type_id, int64_type_id, int128_type_id,
                           uint32_type_id, float32_type_id, float64_type_id, complex_float32_type_id,
                           complex_float64_type_id, void_type_id, string_type_id, fixed_dim_type_id, date_type_id,
                           time_type_id, datetime_type_id> type_ids;

  std::map<type_id_t, callable> children = callable::make_all<is_missing_kernel, type_ids>();
  std::array<callable, 2> dim_children;

  callable self = functional::call<is_missing>(ndt::type("(Any) -> Any"));

  for (auto tp_id : {fixed_dim_type_id, var_dim_type_id}) {
    dim_children[tp_id - fixed_dim_type_id] = functional::elwise(self);
  }

  return functional::dispatch(ndt::type("(Any) -> Any"),
                              [children, dim_children](const ndt::type &DYND_UNUSED(dst_tp), intptr_t DYND_UNUSED(nsrc),
                                                       const ndt::type *src_tp) mutable -> callable & {
                                callable *child = nullptr;
                                if (src_tp[0].get_kind() == option_kind)
                                  child =
                                      &children[src_tp[0].extended<ndt::option_type>()->get_value_type().get_type_id()];
                                else
                                  child = &dim_children[src_tp[0].get_type_id() - fixed_dim_type_id];

                                if (child->is_null()) {
                                  throw std::runtime_error("no child found");
                                }

                                return *child;
                              });
}

DYND_API struct nd::is_missing nd::is_missing;
