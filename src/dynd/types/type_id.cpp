//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/type_registry.hpp>
#include <dynd/types/type_id.hpp>

using namespace std;
using namespace dynd;

std::ostream &dynd::operator<<(std::ostream &o, type_kind_t kind)
{
  switch (kind) {
  case bool_kind:
    return (o << "Bool");
  case sint_kind:
    return (o << "SInt");
  case uint_kind:
    return (o << "UInt");
  case real_kind:
    return (o << "Real");
  case complex_kind:
    return (o << "Complex");
  case string_kind:
    return (o << "String");
  case bytes_kind:
    return (o << "Bytes");
  case void_kind:
    return (o << "Void");
  case datetime_kind:
    return (o << "Datetime");
  case type_kind:
    return (o << "Type");
  case dim_kind:
    return (o << "Dim");
  case struct_kind:
    return (o << "Struct");
  case tuple_kind:
    return (o << "Tuple");
  case expr_kind:
    return (o << "Expr");
  case option_kind:
    return (o << "Option");
  case memory_kind:
    return (o << "Memory");
  case function_kind:
    return (o << "Function");
  case kind_kind:
    return (o << "Kind");
  case pattern_kind:
    return (o << "Pattern");
  case custom_kind:
    return (o << "Custom");
  default:
    return (o << "(unknown kind " << (int)kind << ")");
  }
}

bool dynd::is_base_id_of(type_id_t base_id, type_id_t id)
{
  const ndt::type_info &tp_info = ndt::type_registry[id];
  for (size_t i = 0; i < tp_info.nbases; ++i) {
    if (tp_info.bases[i] == base_id) {
      return true;
    }
  }

  return false;
}
