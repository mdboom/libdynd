//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/types/base_memory_type.hpp>

namespace dynd {

#ifdef DYND_CUDA

class DYND_API cuda_device_type : public base_memory_type {
public:
  cuda_device_type(const ndt::type &element_tp);

  virtual ~cuda_device_type();

  void print_data(std::ostream &o, const char *arrmeta, const char *data) const;

  void print_type(std::ostream &o) const;

  bool operator==(const base_type &rhs) const;

  ndt::type with_replaced_storage_type(const ndt::type &element_tp) const;

  void data_alloc(char **data, size_t size) const;
  void data_zeroinit(char *data, size_t size) const;
  void data_free(char *data) const;
};

DYND_API size_t get_cuda_device_data_alignment(const ndt::type &tp);

namespace ndt {
  inline ndt::type make_cuda_device(const ndt::type &element_tp)
  {
    return ndt::type(new cuda_device_type(element_tp), false);
  }
} // namespace ndt

#endif

} // namespace dynd
