//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#pragma once

#include <dynd/callable.hpp>

namespace dynd {
namespace nd {

  extern DYND_API class callable_registry {
    //    std::map<std::string, callable> m_map;

  public:
    /**
     * Returns a reference to the map of registered callables.
     * NOTE: The internal representation will change, this
     *       function will change.
     */
    DYND_API std::map<std::string, nd::callable> &get_regfunctions();

    /**
      * Looks up a named callable from the registry.
      */
    DYND_API callable &operator[](const std::string &name);
  } callable_registry;

} // namespace dynd::nd
} // namespace dynd
