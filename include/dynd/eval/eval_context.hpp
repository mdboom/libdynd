//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/config.hpp>

#include <dynd/config.hpp>
#include <dynd/types/date_util.hpp>

namespace dynd {
namespace eval {

  struct DYND_API eval_context {
    // Default error mode for computations
    assign_error_mode errmode;

    eval_context() : errmode(assign_error_fractional) {}
  };

  extern DYND_API eval_context default_eval_context;
}
} // namespace dynd::eval
