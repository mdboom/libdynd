//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/types/pow_dimsym_type.hpp>
#include <dynd/types/typevar_type.hpp>
#include <dynd/types/fixed_dim_type.hpp>
#include <dynd/types/typevar_dim_type.hpp>
#include <dynd/types/typevar_constructed_type.hpp>

using namespace std;
using namespace dynd;

ndt::pow_dimsym_type::pow_dimsym_type(const type &base_tp, const std::string &exponent, const type &element_type)
    : base_dim_type(pow_dimsym_id, pattern_kind, element_type, 0, 1, 0, type_flag_symbolic, false), m_base_tp(base_tp),
      m_exponent(exponent)
{
  if (base_tp.is_scalar() || base_tp.extended<base_dim_type>()->get_element_type().get_id() != void_id) {
    stringstream ss;
    ss << "dynd base type for dimensional power symbolic type is not valid: " << base_tp;
    throw type_error(ss.str());
  }
  if (m_exponent.empty()) {
    throw type_error("dynd typevar name cannot be null");
  }
  else if (!is_valid_typevar_name(m_exponent.c_str(), m_exponent.c_str() + m_exponent.size())) {
    stringstream ss;
    ss << "dynd typevar name ";
    print_escaped_utf8_string(ss, m_exponent);
    ss << " is not valid, it must be alphanumeric and begin with a capital";
    throw type_error(ss.str());
  }
}

void ndt::pow_dimsym_type::print_data(std::ostream &DYND_UNUSED(o), const char *DYND_UNUSED(arrmeta),
                                      const char *DYND_UNUSED(data)) const
{
  throw type_error("Cannot store data of typevar type");
}

void ndt::pow_dimsym_type::print_type(std::ostream &o) const
{
  switch (m_base_tp.get_id()) {
  case fixed_dim_id:
    if (m_base_tp.get_kind() == kind_kind) {
      o << "Fixed";
    }
    else {
      o << m_base_tp.extended<fixed_dim_type>()->get_fixed_dim_size();
    }
    break;
  case var_dim_id:
    o << "var";
    break;
  case typevar_dim_id:
    o << m_base_tp.extended<typevar_dim_type>()->get_name();
    break;
  default:
    break;
  }

  o << "**" << m_exponent << " * " << get_element_type();
}

ndt::type ndt::pow_dimsym_type::apply_linear_index(intptr_t DYND_UNUSED(nindices), const irange *DYND_UNUSED(indices),
                                                   size_t DYND_UNUSED(current_i), const type &DYND_UNUSED(root_tp),
                                                   bool DYND_UNUSED(leading_dimension)) const
{
  throw type_error("Cannot store data of typevar type");
}

intptr_t ndt::pow_dimsym_type::apply_linear_index(
    intptr_t DYND_UNUSED(nindices), const irange *DYND_UNUSED(indices), const char *DYND_UNUSED(arrmeta),
    const type &DYND_UNUSED(result_tp), char *DYND_UNUSED(out_arrmeta),
    const intrusive_ptr<memory_block_data> &DYND_UNUSED(embedded_reference), size_t DYND_UNUSED(current_i),
    const type &DYND_UNUSED(root_tp), bool DYND_UNUSED(leading_dimension), char **DYND_UNUSED(inout_data),
    intrusive_ptr<memory_block_data> &DYND_UNUSED(inout_dataref)) const
{
  throw type_error("Cannot store data of typevar type");
}

intptr_t ndt::pow_dimsym_type::get_dim_size(const char *DYND_UNUSED(arrmeta), const char *DYND_UNUSED(data)) const
{
  return -1;
}

bool ndt::pow_dimsym_type::is_lossless_assignment(const type &DYND_UNUSED(dst_tp),
                                                  const type &DYND_UNUSED(src_tp)) const
{
  return false;
}

bool ndt::pow_dimsym_type::operator==(const base_type &rhs) const
{
  if (this == &rhs) {
    return true;
  }
  else if (rhs.get_id() != pow_dimsym_id) {
    return false;
  }
  else {
    const pow_dimsym_type *tvt = static_cast<const pow_dimsym_type *>(&rhs);
    return m_exponent == tvt->m_exponent && m_base_tp == tvt->m_base_tp && m_element_tp == tvt->m_element_tp;
  }
}

void ndt::pow_dimsym_type::arrmeta_default_construct(char *DYND_UNUSED(arrmeta), bool DYND_UNUSED(blockref_alloc)) const
{
  throw type_error("Cannot store data of typevar type");
}

void ndt::pow_dimsym_type::arrmeta_copy_construct(
    char *DYND_UNUSED(dst_arrmeta), const char *DYND_UNUSED(src_arrmeta),
    const intrusive_ptr<memory_block_data> &DYND_UNUSED(embedded_reference)) const
{
  throw type_error("Cannot store data of typevar type");
}

size_t ndt::pow_dimsym_type::arrmeta_copy_construct_onedim(
    char *DYND_UNUSED(dst_arrmeta), const char *DYND_UNUSED(src_arrmeta),
    const intrusive_ptr<memory_block_data> &DYND_UNUSED(embedded_reference)) const
{
  throw type_error("Cannot store data of typevar type");
}

void ndt::pow_dimsym_type::arrmeta_destruct(char *DYND_UNUSED(arrmeta)) const
{
  throw type_error("Cannot store data of typevar type");
}

bool ndt::pow_dimsym_type::match(const char *arrmeta, const type &candidate_tp, const char *candidate_arrmeta,
                                 std::map<std::string, type> &tp_vars) const
{
  if (candidate_tp.get_id() == typevar_constructed_id) {
    return candidate_tp.extended<typevar_constructed_type>()->match(candidate_arrmeta, type(this, true), arrmeta,
                                                                    tp_vars);
  }

  if (candidate_tp.get_id() == pow_dimsym_id) {
    if (m_base_tp.match(arrmeta, candidate_tp.extended<pow_dimsym_type>()->get_base_type(), NULL, tp_vars)) {

      get_element_type().match(arrmeta, candidate_tp.extended<pow_dimsym_type>()->get_element_type(), NULL, tp_vars);
      type &tv_type = tp_vars[candidate_tp.extended<pow_dimsym_type>()->get_exponent()];
      if (tv_type.is_null()) {
        // This typevar hasn't been seen yet
        tv_type = typevar_dim_type::make(candidate_tp.extended<pow_dimsym_type>()->get_exponent(), make_type<void>());
        return true;
      }
      else {
        // Make sure the type matches previous
        // instances of the type var
        return tv_type.get_id() == typevar_dim_id &&
               tv_type.extended<typevar_dim_type>()->get_name() ==
                   candidate_tp.extended<pow_dimsym_type>()->get_exponent();
      }
    }
  }
  else if (candidate_tp.get_ndim() == 0) {
    if (get_element_type().get_ndim() == 0) {
      // Look up to see if the exponent typevar is already matched
      type &tv_type = tp_vars[get_exponent()];
      if (tv_type.is_null()) {
        // Fill in the exponent by the number of dimensions left
        tv_type = make_fixed_dim(0, make_type<void>());
      }
      else if (tv_type.get_id() == fixed_dim_id) {
        // Make sure the exponent already seen matches the number of
        // dimensions left in the concrete type
        if (tv_type.extended<fixed_dim_type>()->get_fixed_dim_size() != 0) {
          return false;
        }
      }
      else {
        // The exponent is always the dim_size inside a fixed_dim_type
        return false;
      }
      return m_element_tp.match(arrmeta, candidate_tp, NULL, tp_vars);
    }
    else {
      return false;
    }
  }

  // Look up to see if the exponent typevar is already matched
  type &tv_type = tp_vars[get_exponent()];
  intptr_t exponent;
  if (tv_type.is_null()) {
    // Fill in the exponent by the number of dimensions left
    exponent = candidate_tp.get_ndim() - get_element_type().get_ndim();
    tv_type = make_fixed_dim(exponent, make_type<void>());
  }
  else if (tv_type.get_id() == fixed_dim_id) {
    // Make sure the exponent already seen matches the number of
    // dimensions left in the concrete type
    exponent = tv_type.extended<fixed_dim_type>()->get_fixed_dim_size();
    if (exponent != candidate_tp.get_ndim() - get_element_type().get_ndim()) {
      return false;
    }
  }
  else {
    // The exponent is always the dim_size inside a fixed_dim_type
    return false;
  }
  // If the exponent is zero, the base doesn't matter, just match the rest
  if (exponent == 0) {
    return m_element_tp.match(arrmeta, candidate_tp, NULL, tp_vars);
  }
  else if (exponent < 0) {
    return false;
  }
  // Get the base type
  type base_tp = get_base_type();
  if (base_tp.get_id() == typevar_dim_id) {
    type &btv_type = tp_vars[base_tp.extended<typevar_dim_type>()->get_name()];
    if (btv_type.is_null()) {
      // We haven't seen this typevar yet, set it to the concrete's
      // dimension type
      btv_type = candidate_tp;
      base_tp = candidate_tp;
    }
    else if (btv_type.get_ndim() > 0 && btv_type.get_id() != dim_fragment_id) {
      // Continue matching after substituting in the typevar for
      // the base type
      base_tp = btv_type;
    }
    else {
      // Doesn't match if the typevar has a dim fragment or dtype in it
      return false;
    }
  }
  // Now make sure the base_tp is repeated the right number of times
  type concrete_subtype = candidate_tp;
  switch (base_tp.get_id()) {
  case fixed_dim_id: {
    if (base_tp.get_kind() == kind_kind) {
      for (intptr_t i = 0; i < exponent; ++i) {
        switch (concrete_subtype.get_id()) {
        case fixed_dim_id:
          concrete_subtype = concrete_subtype.extended<base_dim_type>()->get_element_type();
          break;
        default:
          return false;
        }
      }
      break;
    }
    else {
      intptr_t dim_size = base_tp.extended<fixed_dim_type>()->get_fixed_dim_size();
      for (intptr_t i = 0; i < exponent; ++i) {
        if (concrete_subtype.get_id() == fixed_dim_id &&
            concrete_subtype.extended<fixed_dim_type>()->get_fixed_dim_size() == dim_size) {
          concrete_subtype = concrete_subtype.extended<base_dim_type>()->get_element_type();
        }
        else {
          return false;
        }
      }
      break;
    }
  }
  case var_dim_id:
    for (intptr_t i = 0; i < exponent; ++i) {
      if (concrete_subtype.get_id() == var_dim_id) {
        concrete_subtype = concrete_subtype.extended<base_dim_type>()->get_element_type();
      }
    }
    break;
  default:
    return false;
  }
  return m_element_tp.match(arrmeta, concrete_subtype, NULL, tp_vars);
}

/*
static nd::array property_get_name(const ndt::type& tp) {
    return tp.extended<typevar_dim_type>()->get_name();
}

static ndt::type property_get_element_type(const ndt::type& dt) {
    return dt.extended<typevar_dim_type>()->get_element_type();
}

void typevar_dim_type::get_dynamic_type_properties(
    const std::pair<std::string, gfunc::callable> **out_properties,
    size_t *out_count) const
{
    static pair<string, gfunc::callable> type_properties[] = {
        pair<string, gfunc::callable>(
            "name", gfunc::make_callable(&property_get_name, "self")),
        pair<string, gfunc::callable>(
            "element_type",
            gfunc::make_callable(&property_get_element_type, "self")), };

    *out_properties = type_properties;
    *out_count = sizeof(type_properties) / sizeof(type_properties[0]);
}
*/

ndt::type ndt::pow_dimsym_type::with_element_type(const type &element_tp) const
{
  return make_pow_dimsym(m_base_tp, m_exponent, element_tp);
}
