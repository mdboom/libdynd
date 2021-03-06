//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/kernels/apply.hpp>
#include <dynd/kernels/base_kernel.hpp>

namespace dynd {
namespace nd {
  namespace functional {

    template <typename T, typename mem_func_type, typename R, typename A, typename I, typename K, typename J>
    struct apply_member_function_ck;

#define APPLY_MEMBER_FUNCTION_CK(...)                                                                                  \
  template <typename T, typename mem_func_type, typename R, typename... A, size_t... I, typename... K, size_t... J>    \
  struct apply_member_function_ck<T *, mem_func_type, R, type_sequence<A...>, index_sequence<I...>,                    \
                                  type_sequence<K...>, index_sequence<J...>>                                           \
      : base_kernel<apply_member_function_ck<T *, mem_func_type, R, type_sequence<A...>, index_sequence<I...>,         \
                                             type_sequence<K...>, index_sequence<J...>>,                               \
                    sizeof...(A)>,                                                                                     \
        apply_args<type_sequence<A...>, index_sequence<I...>>,                                                         \
        apply_kwds<type_sequence<K...>, index_sequence<J...>> {                                                        \
    typedef apply_member_function_ck self_type;                                                                        \
    typedef apply_args<type_sequence<A...>, index_sequence<I...>> args_type;                                           \
    typedef apply_kwds<type_sequence<K...>, index_sequence<J...>> kwds_type;                                           \
    typedef std::pair<T *, mem_func_type> data_type;                                                                   \
                                                                                                                       \
    static const kernel_request_t kernreq = kernel_request_call;                                                       \
                                                                                                                       \
    T *obj;                                                                                                            \
    mem_func_type mem_func;                                                                                            \
                                                                                                                       \
    __VA_ARGS__ apply_member_function_ck(T *obj, mem_func_type mem_func, args_type args, kwds_type kwds)               \
        : args_type(args), kwds_type(kwds), obj(obj), mem_func(mem_func)                                               \
    {                                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    __VA_ARGS__ void single(char *dst, char *const *DYND_IGNORE_UNUSED(src))                                           \
    {                                                                                                                  \
      *reinterpret_cast<R *>(dst) = (obj->*mem_func)(apply_arg<A, I>::get(src[I])..., apply_kwd<K, J>::get()...);      \
    }                                                                                                                  \
                                                                                                                       \
    __VA_ARGS__ void strided(char *dst, intptr_t dst_stride, char *const *DYND_IGNORE_UNUSED(src_copy),                \
                             const intptr_t *DYND_IGNORE_UNUSED(src_stride), size_t count)                             \
    {                                                                                                                  \
      dynd::detail::array_wrapper<char *, sizeof...(A)> src;                                                           \
                                                                                                                       \
      dst += DYND_THREAD_ID(0) * dst_stride;                                                                           \
      for (size_t j = 0; j != sizeof...(A); ++j) {                                                                     \
        src[j] = src_copy[j] + DYND_THREAD_ID(0) * src_stride[j];                                                      \
      }                                                                                                                \
                                                                                                                       \
      for (std::ptrdiff_t i = DYND_THREAD_ID(0); i < (std::ptrdiff_t)count; i += DYND_THREAD_COUNT(0)) {               \
        *reinterpret_cast<R *>(dst) = (obj->*mem_func)(apply_arg<A, I>::get(src[I])..., apply_kwd<K, J>::get()...);    \
        dst += DYND_THREAD_COUNT(0) * dst_stride;                                                                      \
        for (size_t j = 0; j != sizeof...(A); ++j) {                                                                   \
          src[j] += DYND_THREAD_COUNT(0) * src_stride[j];                                                              \
        }                                                                                                              \
      }                                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    static intptr_t instantiate_without_cuda_launch(char *static_data, char *DYND_UNUSED(data), void *ckb,             \
                                                    intptr_t ckb_offset, const ndt::type &DYND_UNUSED(dst_tp),         \
                                                    const char *DYND_UNUSED(dst_arrmeta), intptr_t DYND_UNUSED(nsrc),  \
                                                    const ndt::type *src_tp, const char *const *src_arrmeta,           \
                                                    kernel_request_t kernreq, intptr_t nkwd, const nd::array *kwds,    \
                                                    const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))      \
    {                                                                                                                  \
      self_type::make(ckb, kernreq, ckb_offset, reinterpret_cast<data_type *>(static_data)->first,                     \
                      dynd::detail::make_value_wrapper(reinterpret_cast<data_type *>(static_data)->second),            \
                      args_type(src_tp, src_arrmeta, kwds), kwds_type(nkwd, kwds));                                    \
      return ckb_offset;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
    static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb,                    \
                                intptr_t ckb_offset, const ndt::type &dst_tp, const char *dst_arrmeta, intptr_t nsrc,  \
                                const ndt::type *src_tp, const char *const *src_arrmeta, kernel_request_t kernreq,     \
                                intptr_t nkwd, const nd::array *kwds,                                                  \
                                const std::map<std::string, ndt::type> &tp_vars);                                      \
  };                                                                                                                   \
                                                                                                                       \
  template <typename T, typename mem_func_type, typename... A, size_t... I, typename... K, size_t... J>                \
  struct apply_member_function_ck<T *, mem_func_type, void, type_sequence<A...>, index_sequence<I...>,                 \
                                  type_sequence<K...>, index_sequence<J...>>                                           \
      : base_kernel<apply_member_function_ck<T *, mem_func_type, void, type_sequence<A...>, index_sequence<I...>,      \
                                             type_sequence<K...>, index_sequence<J...>>,                               \
                    sizeof...(A)>,                                                                                     \
        apply_args<type_sequence<A...>, index_sequence<I...>>,                                                         \
        apply_kwds<type_sequence<K...>, index_sequence<J...>> {                                                        \
    typedef apply_member_function_ck self_type;                                                                        \
    typedef apply_args<type_sequence<A...>, index_sequence<I...>> args_type;                                           \
    typedef apply_kwds<type_sequence<K...>, index_sequence<J...>> kwds_type;                                           \
    typedef std::pair<T *, mem_func_type> data_type;                                                                   \
                                                                                                                       \
    static const kernel_request_t kernreq = kernel_request_call;                                                       \
                                                                                                                       \
    T *obj;                                                                                                            \
    mem_func_type mem_func;                                                                                            \
                                                                                                                       \
    __VA_ARGS__ apply_member_function_ck(T *obj, mem_func_type mem_func, args_type args, kwds_type kwds)               \
        : args_type(args), kwds_type(kwds), obj(obj), mem_func(mem_func)                                               \
    {                                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    __VA_ARGS__ void single(char *DYND_UNUSED(dst), char *const *DYND_IGNORE_UNUSED(src))                              \
    {                                                                                                                  \
      (obj->*mem_func)(apply_arg<A, I>::get(src[I])..., apply_kwd<K, J>::get()...);                                    \
    }                                                                                                                  \
                                                                                                                       \
    __VA_ARGS__ void strided(char *DYND_UNUSED(dst), intptr_t DYND_UNUSED(dst_stride),                                 \
                             char *const *DYND_IGNORE_UNUSED(src_copy),                                                \
                             const intptr_t *DYND_IGNORE_UNUSED(src_stride), size_t count)                             \
    {                                                                                                                  \
      dynd::detail::array_wrapper<char *, sizeof...(A)> src;                                                           \
                                                                                                                       \
      for (size_t j = 0; j != sizeof...(A); ++j) {                                                                     \
        src[j] = src_copy[j] + DYND_THREAD_ID(0) * src_stride[j];                                                      \
      }                                                                                                                \
                                                                                                                       \
      for (std::ptrdiff_t i = DYND_THREAD_ID(0); i < (std::ptrdiff_t)count; i += DYND_THREAD_COUNT(0)) {               \
        (obj->*mem_func)(apply_arg<A, I>::get(src[I])..., apply_kwd<K, J>::get()...);                                  \
        for (size_t j = 0; j != sizeof...(A); ++j) {                                                                   \
          src[j] += DYND_THREAD_COUNT(0) * src_stride[j];                                                              \
        }                                                                                                              \
      }                                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    static intptr_t instantiate_without_cuda_launch(char *static_data, char *DYND_UNUSED(data), void *ckb,             \
                                                    intptr_t ckb_offset, const ndt::type &DYND_UNUSED(dst_tp),         \
                                                    const char *DYND_UNUSED(dst_arrmeta), intptr_t DYND_UNUSED(nsrc),  \
                                                    const ndt::type *src_tp, const char *const *src_arrmeta,           \
                                                    kernel_request_t kernreq, intptr_t nkwd, const nd::array *kwds,    \
                                                    const std::map<std::string, ndt::type> &DYND_UNUSED(tp_vars))      \
    {                                                                                                                  \
      self_type::make(ckb, kernreq, ckb_offset, reinterpret_cast<data_type *>(static_data)->first,                     \
                      dynd::detail::make_value_wrapper(reinterpret_cast<data_type *>(static_data)->second),            \
                      args_type(src_tp, src_arrmeta, kwds), kwds_type(nkwd, kwds));                                    \
      return ckb_offset;                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
    static intptr_t instantiate(char *DYND_UNUSED(static_data), char *DYND_UNUSED(data), void *ckb,                    \
                                intptr_t ckb_offset, const ndt::type &dst_tp, const char *dst_arrmeta, intptr_t nsrc,  \
                                const ndt::type *src_tp, const char *const *src_arrmeta, kernel_request_t kernreq,     \
                                intptr_t nkwd, const nd::array *kwds,                                                  \
                                const std::map<std::string, ndt::type> &tp_vars);                                      \
  }

    APPLY_MEMBER_FUNCTION_CK();

    template <typename T, typename mem_func_type, typename R, typename... A, size_t... I, typename... K, size_t... J>
    intptr_t
    apply_member_function_ck<T *, mem_func_type, R, type_sequence<A...>, index_sequence<I...>, type_sequence<K...>,
                             index_sequence<J...>>::instantiate(char *static_data, char *DYND_UNUSED(data), void *ckb,
                                                                intptr_t ckb_offset, const ndt::type &dst_tp,
                                                                const char *dst_arrmeta, intptr_t nsrc,
                                                                const ndt::type *src_tp, const char *const *src_arrmeta,
                                                                kernel_request_t kernreq, intptr_t nkwd,
                                                                const nd::array *kwds,
                                                                const std::map<std::string, ndt::type> &tp_vars)
    {
      return instantiate_without_cuda_launch(static_data, NULL, ckb, ckb_offset, dst_tp, dst_arrmeta, nsrc, src_tp,
                                             src_arrmeta, kernreq, nkwd, kwds, tp_vars);
    }

    template <typename T, typename mem_func_type, typename... A, size_t... I, typename... K, size_t... J>
    intptr_t
    apply_member_function_ck<T *, mem_func_type, void, type_sequence<A...>, index_sequence<I...>, type_sequence<K...>,
                             index_sequence<J...>>::instantiate(char *static_data, char *DYND_UNUSED(data), void *ckb,
                                                                intptr_t ckb_offset, const ndt::type &dst_tp,
                                                                const char *dst_arrmeta, intptr_t nsrc,
                                                                const ndt::type *src_tp, const char *const *src_arrmeta,
                                                                kernel_request_t kernreq, intptr_t nkwd,
                                                                const nd::array *kwds,
                                                                const std::map<std::string, ndt::type> &tp_vars)
    {
      return instantiate_without_cuda_launch(static_data, NULL, ckb, ckb_offset, dst_tp, dst_arrmeta, nsrc, src_tp,
                                             src_arrmeta, kernreq, nkwd, kwds, tp_vars);
    }

#undef APPLY_MEMBER_FUNCTION_CK

    template <typename T, typename mem_func_type, int N>
    using as_apply_member_function_ck =
        apply_member_function_ck<T, mem_func_type, typename return_of<mem_func_type>::type,
                                 as_apply_arg_sequence<mem_func_type, N>, make_index_sequence<N>,
                                 as_apply_kwd_sequence<mem_func_type, N>,
                                 make_index_sequence<arity_of<mem_func_type>::value - N>>;

  } // namespace dynd::nd::functional
} // namespace dynd::nd
} // namespace dynd
