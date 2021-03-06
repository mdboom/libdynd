//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/kernels/base_kernel.hpp>

namespace dynd {
namespace nd {

  template <type_id_t RealTypeID>
  struct real_kernel : base_kernel<real_kernel<RealTypeID>, 1> {
    typedef typename type_of<RealTypeID>::type real_type;
    typedef complex<typename type_of<RealTypeID>::type> complex_type;

    void single(char *dst, char *const *src)
    {
      *reinterpret_cast<real_type *>(dst) = reinterpret_cast<complex_type *>(src[0])->real();
    }
  };

} // namespace dynd::nd

namespace ndt {

  template <type_id_t RealTypeID>
  struct traits<nd::real_kernel<RealTypeID>> {
    static type equivalent()
    {
      return callable_type::make(make_type<typename nd::real_kernel<RealTypeID>::real_type>(),
                                 {make_type<typename nd::real_kernel<RealTypeID>::complex_type>()});
    }
  };

} // namespace dynd::ndt
} // namespace dynd
