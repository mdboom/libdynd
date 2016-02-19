//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/type.hpp>
#include <dynd/type_registry.hpp>
#include <dynd/types/base_dim_type.hpp>
#include <dynd/types/base_memory_type.hpp>
#include <dynd/types/fixed_dim_type.hpp>
#include <dynd/types/var_dim_type.hpp>
#include <dynd/exceptions.hpp>
#include <dynd/callable.hpp>
#include <dynd/types/adapt_type.hpp>
#include <dynd/types/option_type.hpp>
#include <dynd/types/datashape_parser.hpp>
#include <dynd/types/any_kind_type.hpp>
#include <dynd/types/bytes_type.hpp>
#include <dynd/types/categorical_kind_type.hpp>
#include <dynd/types/char_type.hpp>
#include <dynd/types/date_type.hpp>
#include <dynd/types/fixed_bytes_kind_type.hpp>
#include <dynd/types/fixed_string_kind_type.hpp>
#include <dynd/types/var_dim_type.hpp>
#include <dynd/kernels/imag_kernel.hpp>
#include <dynd/kernels/conj_kernel.hpp>
#include <dynd/func/elwise.hpp>
#include <dynd/func/complex.hpp>

#include <sstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <iomanip>

using namespace std;
using namespace dynd;

namespace dynd {
namespace ndt {
  namespace registry {

    static struct {
      std::string name;
      type_make_t func;
    } data[101];
    static size_t size = dim_fragment_id + 1;

  } // namespace dynd::ndt::registry
} // namespace dynd::ndt
} // namespace dynd

char *dynd::iterdata_broadcasting_terminator_incr(iterdata_common *iterdata, intptr_t DYND_UNUSED(level))
{
  // This repeats the same data over and over again, broadcasting additional
  // leftmost dimensions
  iterdata_broadcasting_terminator *id = reinterpret_cast<iterdata_broadcasting_terminator *>(iterdata);
  return id->data;
}

char *dynd::iterdata_broadcasting_terminator_adv(iterdata_common *iterdata, intptr_t DYND_UNUSED(level),
                                                 intptr_t DYND_UNUSED(i))
{
  // This repeats the same data over and over again, broadcasting additional
  // leftmost dimensions
  iterdata_broadcasting_terminator *id = reinterpret_cast<iterdata_broadcasting_terminator *>(iterdata);
  return id->data;
}

char *dynd::iterdata_broadcasting_terminator_reset(iterdata_common *iterdata, char *data, intptr_t DYND_UNUSED(level))
{
  iterdata_broadcasting_terminator *id = reinterpret_cast<iterdata_broadcasting_terminator *>(iterdata);
  id->data = data;
  return data;
}

ndt::type::type(type_id_t id) : type(type_registry[id].kind_tp) {}

ndt::type::type(const std::string &rep) { type_from_datashape(rep).swap(*this); }

ndt::type::type(const char *rep_begin, const char *rep_end) { type_from_datashape(rep_begin, rep_end).swap(*this); }

ndt::type ndt::type::at_array(int nindices, const irange *indices) const
{
  if (this->is_builtin()) {
    if (nindices == 0) {
      return *this;
    }
    else {
      throw too_many_indices(*this, nindices, 0);
    }
  }
  else {
    return m_ptr->apply_linear_index(nindices, indices, 0, *this, true);
  }
}

bool ndt::type::match(const char *arrmeta, const ndt::type &candidate_tp, const char *candidate_arrmeta,
                      std::map<std::string, ndt::type> &tp_vars) const
{
  // A type being matched against itself works for both type id and more
  // complicated types
  if (extended() == candidate_tp.extended()) {
    return true;
  }

  // Builtin types only match against themselves
  if (is_builtin()) {
    return false;
  }

  return extended()->match(arrmeta, candidate_tp, candidate_arrmeta, tp_vars);
}

bool ndt::type::match(const char *arrmeta, const ndt::type &candidate_tp, const char *candidate_arrmeta) const
{
  std::map<std::string, ndt::type> tp_vars;
  return match(arrmeta, candidate_tp, candidate_arrmeta, tp_vars);
}

bool ndt::type::match(const ndt::type &candidate_tp, std::map<std::string, ndt::type> &tp_vars) const
{
  return match(NULL, candidate_tp, NULL, tp_vars);
}

bool ndt::type::match(const ndt::type &candidate_tp) const
{
  std::map<std::string, ndt::type> tp_vars;
  return match(candidate_tp, tp_vars);
}

nd::array ndt::type::p(const char *name) const
{
  map<std::string, nd::callable> properties(get_properties());
  if (nd::callable &p = properties[name]) {
    return p(*this);
  }

  stringstream ss;
  ss << "type does not have property " << name;
  throw runtime_error(ss.str());
}

nd::array ndt::type::p(const std::string &name) const { return p(name.c_str()); }

ndt::type ndt::type::apply_linear_index(intptr_t nindices, const irange *indices, size_t current_i,
                                        const ndt::type &root_tp, bool leading_dimension) const
{
  if (is_builtin()) {
    if (nindices == 0) {
      return *this;
    }
    else {
      throw too_many_indices(*this, nindices + current_i, current_i);
    }
  }
  else {
    return m_ptr->apply_linear_index(nindices, indices, current_i, root_tp, leading_dimension);
  }
}

namespace {
struct replace_scalar_type_extra {
  replace_scalar_type_extra(const ndt::type &dt) : scalar_tp(dt) {}
  const ndt::type &scalar_tp;
};
static void replace_scalar_types(const ndt::type &dt, intptr_t DYND_UNUSED(arrmeta_offset), void *extra,
                                 ndt::type &out_transformed_tp, bool &out_was_transformed)
{
//  const replace_scalar_type_extra *e = reinterpret_cast<const replace_scalar_type_extra *>(extra);
  if (!dt.is_indexable()) {
    throw std::runtime_error("trying to make convert_type");
//    out_transformed_tp = ndt::convert_type::make(e->scalar_tp, dt);
    out_was_transformed = true;
  }
  else {
    dt.extended()->transform_child_types(&replace_scalar_types, 0, extra, out_transformed_tp, out_was_transformed);
  }
}
} // anonymous namespace

ndt::type ndt::type::with_replaced_scalar_types(const ndt::type &scalar_tp) const
{
  ndt::type result;
  bool was_transformed;
  replace_scalar_type_extra extra(scalar_tp);
  replace_scalar_types(*this, 0, &extra, result, was_transformed);
  return result;
}

namespace {
struct replace_dtype_extra {
  replace_dtype_extra(const ndt::type &replacement_tp, intptr_t replace_ndim)
      : m_replacement_tp(replacement_tp), m_replace_ndim(replace_ndim)
  {
  }
  const ndt::type &m_replacement_tp;
  intptr_t m_replace_ndim;
};
static void replace_dtype(const ndt::type &tp, intptr_t DYND_UNUSED(arrmeta_offset), void *extra,
                          ndt::type &out_transformed_tp, bool &out_was_transformed)
{
  const replace_dtype_extra *e = reinterpret_cast<const replace_dtype_extra *>(extra);
  if (tp.get_ndim() == e->m_replace_ndim) {
    out_transformed_tp = e->m_replacement_tp;
    out_was_transformed = true;
  }
  else {
    tp.extended()->transform_child_types(&replace_dtype, 0, extra, out_transformed_tp, out_was_transformed);
  }
}
} // anonymous namespace

ndt::type ndt::type::with_replaced_dtype(const ndt::type &replacement_tp, intptr_t replace_ndim) const
{
  ndt::type result;
  bool was_transformed;
  replace_dtype_extra extra(replacement_tp, replace_ndim);
  replace_dtype(*this, 0, &extra, result, was_transformed);
  return result;
}

ndt::type ndt::type::without_memory_type() const
{
  if (get_kind() == memory_kind) {
    return extended<base_memory_type>()->get_element_type();
  }
  else {
    return *this;
  }
}

const ndt::type &ndt::type::storage_type() const
{
  // Only expr_kind types have different storage_type
  if (is_builtin() || m_ptr->get_kind() != expr_kind) {
    return *this;
  }
  else if (get_id() == adapt_id) {
    return extended<ndt::adapt_type>()->get_storage_type();
  }
  else {
    // Follow the operand type chain to get the storage type
    const type *dt = &static_cast<const base_expr_type *>(m_ptr)->get_operand_type();
    while (dt->get_kind() == expr_kind) {
      dt = &static_cast<const base_expr_type *>(dt->m_ptr)->get_operand_type();
    }
    return *dt;
  }
}

const ndt::type &ndt::type::value_type() const
{
  // Only expr_kind types have different value_type
  if (is_builtin() || m_ptr->get_kind() != expr_kind) {
    return *this;
  }
  else if (get_id() == adapt_id) {
    return extended<ndt::adapt_type>()->get_value_type();
  }
  else {
    // All chaining happens in the operand_type
    return static_cast<const base_expr_type *>(m_ptr)->get_value_type();
  }
}

ndt::type ndt::type::with_new_axis(intptr_t i, intptr_t new_ndim) const
{
  ndt::type tp = without_memory_type();

  tp = tp.with_replaced_dtype(ndt::make_fixed_dim(1, tp.get_type_at_dimension(NULL, i), new_ndim), tp.get_ndim() - i);
  if (get_kind() == memory_kind) {
    tp = extended<base_memory_type>()->with_replaced_storage_type(tp);
  }

  return tp;
}

intptr_t ndt::type::get_dim_size(const char *arrmeta, const char *data) const
{
  if (get_kind() == dim_kind) {
    return static_cast<const base_dim_type *>(get())->get_dim_size(arrmeta, data);
  }
  else if (get_kind() == struct_kind) {
    return static_cast<const struct_type *>(get())->get_field_count();
  }
  else if (get_ndim() > 0) {
    intptr_t dim_size = -1;
    get()->get_shape(1, 0, &dim_size, arrmeta, data);
    if (dim_size >= 0) {
      return dim_size;
    }
  }

  std::stringstream ss;
  ss << "Scalar dynd array of type " << *this << " has no length";
  throw std::invalid_argument(ss.str());
}

intptr_t ndt::type::get_size(const char *arrmeta) const
{
  if (is_scalar()) {
    return 1;
  }

  return extended<base_dim_type>()->get_size(arrmeta);
}

bool ndt::type::get_as_strided(const char *arrmeta, intptr_t *out_dim_size, intptr_t *out_stride, ndt::type *out_el_tp,
                               const char **out_el_arrmeta) const
{
  if (get_kind() == memory_kind) {
    bool res = without_memory_type().get_as_strided(arrmeta, out_dim_size, out_stride, out_el_tp, out_el_arrmeta);
    *out_el_tp = extended<base_memory_type>()->with_replaced_storage_type(*out_el_tp);
    return res;
  }

  if (get_strided_ndim() >= 1) {
    *out_dim_size = reinterpret_cast<const size_stride_t *>(arrmeta)->dim_size;
    *out_stride = reinterpret_cast<const size_stride_t *>(arrmeta)->stride;
    *out_el_tp = extended<base_dim_type>()->get_element_type();
    *out_el_arrmeta = arrmeta + sizeof(fixed_dim_type_arrmeta);
    return true;
  }
  else {
    return false;
  }
}

std::map<std::string, nd::callable> ndt::type::get_properties() const
{
  std::map<std::string, nd::callable> properties;
  if (!is_builtin()) {
    return m_ptr->get_dynamic_type_properties();
  }

  return properties;
}

std::map<std::string, nd::callable> ndt::type::get_functions() const
{
  std::map<std::string, nd::callable> functions;
  if (!is_builtin()) {
    return m_ptr->get_dynamic_type_functions();
  }

  return functions;
}

namespace {

const std::map<std::string, nd::callable> &complex32_array_properties()
{
  static const std::map<std::string, nd::callable> complex_array_properties{
      {"real", nd::real::get()}, {"imag", nd::imag::get()}, {"conj", nd::conj::get()}};

  return complex_array_properties;
}

const std::map<std::string, nd::callable> &complex64_array_properties()
{
  static const std::map<std::string, nd::callable> complex_array_properties{
      {"real", nd::real::get()}, {"imag", nd::imag::get()}, {"conj", nd::conj::get()}};

  return complex_array_properties;
}

} // anonymous namespace

std::map<std::string, nd::callable> ndt::type::get_array_properties() const
{
  std::map<std::string, nd::callable> array_properties;
  if (is_builtin()) {
    switch (static_cast<type_id_t>(reinterpret_cast<intptr_t>(m_ptr))) {
    case complex_float32_id:
      array_properties = complex32_array_properties();
      break;
    case complex_float64_id:
      array_properties = complex64_array_properties();
      break;
    default:
      break;
    }
  }
  else {
    array_properties = m_ptr->get_dynamic_array_properties();
  }

  return array_properties;
}

std::map<std::string, nd::callable> ndt::type::get_array_functions() const
{
  std::map<std::string, nd::callable> array_functions;
  if (!is_builtin()) {
    array_functions = m_ptr->get_dynamic_array_functions();
  }

  return array_functions;
}

bool ndt::type::get_as_strided(const char *arrmeta, intptr_t ndim, const size_stride_t **out_size_stride,
                               ndt::type *out_el_tp, const char **out_el_arrmeta) const
{
  if (get_strided_ndim() >= ndim) {
    *out_size_stride = reinterpret_cast<const size_stride_t *>(arrmeta);
    *out_el_arrmeta = arrmeta + ndim * sizeof(fixed_dim_type_arrmeta);
    *out_el_tp = *this;
    while (ndim-- > 0) {
      *out_el_tp = out_el_tp->extended<base_dim_type>()->get_element_type();
    }
    return true;
  }
  else {
    return false;
  }
}
bool ndt::type::data_layout_compatible_with(const ndt::type &rhs) const
{
  if (extended() == rhs.extended()) {
    // If they're trivially identical, quickly return true
    return true;
  }
  if (get_data_size() != rhs.get_data_size() || get_arrmeta_size() != rhs.get_arrmeta_size()) {
    // The size of the data and arrmeta must be the same
    return false;
  }
  if (get_arrmeta_size() == 0 && is_pod() && rhs.is_pod()) {
    // If both are POD with no arrmeta, then they're compatible
    return true;
  }
  if (get_kind() == expr_kind || rhs.get_kind() == expr_kind) {
    // If either is an expression type, check compatibility with
    // the storage types
    return storage_type().data_layout_compatible_with(rhs.storage_type());
  }
  // Rules for the rest of the types
  switch (get_id()) {
  case string_id:
  case bytes_id:
    switch (rhs.get_id()) {
    case string_id:
    case bytes_id:
      // All of string, bytes, json are compatible
      return true;
    default:
      return false;
    }
  case fixed_dim_id:
    if (rhs.get_id() == fixed_dim_id) {
      return extended<fixed_dim_type>()->get_fixed_dim_size() == rhs.extended<fixed_dim_type>()->get_fixed_dim_size() &&
             extended<fixed_dim_type>()->get_element_type().data_layout_compatible_with(
                 rhs.extended<fixed_dim_type>()->get_element_type());
    }
    break;
  case var_dim_id:
    // For var dimensions, it's data layout
    // compatible if the element is
    if (rhs.get_id() == var_dim_id) {
      const base_dim_type *budd = static_cast<const base_dim_type *>(extended());
      const base_dim_type *rhs_budd = rhs.extended<base_dim_type>();
      return budd->get_element_type().data_layout_compatible_with(rhs_budd->get_element_type());
    }
    break;
  default:
    break;
  }
  return false;
}

std::ostream &dynd::operator<<(std::ostream &o, type_id_t tid)
{
  switch (tid) {
  case uninitialized_id:
    return (o << "uninitialized");
  case bool_id:
    return (o << "bool");
  case int8_id:
    return (o << "int8");
  case int16_id:
    return (o << "int16");
  case int32_id:
    return (o << "int32");
  case int64_id:
    return (o << "int64");
  case int128_id:
    return (o << "int128");
  case uint8_id:
    return (o << "uint8");
  case uint16_id:
    return (o << "uint16");
  case uint32_id:
    return (o << "uint32");
  case uint64_id:
    return (o << "uint64");
  case uint128_id:
    return (o << "uint128");
  case float16_id:
    return (o << "float16");
  case float32_id:
    return (o << "float32");
  case float64_id:
    return (o << "float64");
  case float128_id:
    return (o << "float128");
  case complex_float32_id:
    return (o << "complex_float32");
  case complex_float64_id:
    return (o << "complex_float64");
  case void_id:
    return (o << "void");
  case pointer_id:
    return (o << "pointer");
  case bytes_id:
    return (o << "bytes");
  case fixed_bytes_id:
    return (o << "fixed_bytes");
  case string_id:
    return (o << "string");
  case fixed_string_id:
    return (o << "fixed_string");
  case categorical_id:
    return (o << "categorical");
  case fixed_dim_id:
    return (o << "fixed_dim");
  case var_dim_id:
    return (o << "var_dim");
  case struct_id:
    return (o << "struct");
  case tuple_id:
    return (o << "tuple");
  case c_contiguous_id:
    return (o << "C");
  case option_id:
    return (o << "option");
  case adapt_id:
    return o << "adapt";
  case kind_sym_id:
    return (o << "kind_sym");
  case int_sym_id:
    return (o << "int_sym");
  case expr_id:
    return (o << "expr");
  case type_id:
    return (o << "type");
  case callable_id:
    return (o << "callable");
  case typevar_id:
    return (o << "typevar");
  case typevar_constructed_id:
    return (o << "typevar_constructed");
  case typevar_dim_id:
    return (o << "typevar_dim");
  case ellipsis_dim_id:
    return (o << "ellipsis_dim");
  default:
    return o << ndt::registry::data[tid].name;
  }
}

std::ostream &dynd::ndt::operator<<(std::ostream &o, const ndt::type &rhs)
{
  switch (rhs.get_id()) {
  case uninitialized_id:
    o << "uninitialized";
    break;
  case bool_id:
    o << "bool";
    break;
  case int8_id:
    o << "int8";
    break;
  case int16_id:
    o << "int16";
    break;
  case int32_id:
    o << "int32";
    break;
  case int64_id:
    o << "int64";
    break;
  case int128_id:
    o << "int128";
    break;
  case uint8_id:
    o << "uint8";
    break;
  case uint16_id:
    o << "uint16";
    break;
  case uint32_id:
    o << "uint32";
    break;
  case uint64_id:
    o << "uint64";
    break;
  case uint128_id:
    o << "uint128";
    break;
  case float16_id:
    o << "float16";
    break;
  case float32_id:
    o << "float32";
    break;
  case float64_id:
    o << "float64";
    break;
  case float128_id:
    o << "float128";
    break;
  case complex_float32_id:
    o << "complex[float32]";
    break;
  case complex_float64_id:
    o << "complex[float64]";
    break;
  case void_id:
    o << "void";
    break;
  default:
    rhs.extended()->print_type(o);
    break;
  }

  return o;
}

ndt::type ndt::make_type(intptr_t ndim, const intptr_t *shape, const ndt::type &dtp)
{
  if (ndim > 0) {
    ndt::type result_tp =
        shape[ndim - 1] >= 0 ? ndt::make_fixed_dim(shape[ndim - 1], dtp) : ndt::var_dim_type::make(dtp);
    for (intptr_t i = ndim - 2; i >= 0; --i) {
      if (shape[i] >= 0) {
        result_tp = ndt::make_fixed_dim(shape[i], result_tp);
      }
      else {
        result_tp = ndt::var_dim_type::make(result_tp);
      }
    }
    return result_tp;
  }
  else {
    return dtp;
  }
}

ndt::type ndt::make_type(intptr_t ndim, const intptr_t *shape, const ndt::type &dtp, bool &out_any_var)
{
  if (ndim > 0) {
    ndt::type result_tp = dtp;
    for (intptr_t i = ndim - 1; i >= 0; --i) {
      if (shape[i] >= 0) {
        result_tp = ndt::make_fixed_dim(shape[i], result_tp);
      }
      else {
        result_tp = ndt::var_dim_type::make(result_tp);
        out_any_var = true;
      }
    }
    return result_tp;
  }
  else {
    return dtp;
  }
}

type_id_t ndt::register_type(const std::string &name, type_make_t make)
{
  registry::data[registry::size].name = name;
  registry::data[registry::size].func = make;

  return static_cast<type_id_t>(registry::size++);
}

ndt::type ndt::type::make(type_id_t tp_id, const nd::array &args) { return registry::data[tp_id].func(tp_id, args); }

template <class T, class Tas>
static void print_as(std::ostream &o, const char *data)
{
  T value;
  memcpy(&value, data, sizeof(value));
  o << static_cast<Tas>(value);
}

void dynd::hexadecimal_print(std::ostream &o, char value)
{
  static char hexadecimal[] = "0123456789abcdef";
  unsigned char v = (unsigned char)value;
  o << hexadecimal[v >> 4] << hexadecimal[v & 0x0f];
}

void dynd::hexadecimal_print(std::ostream &o, unsigned char value) { hexadecimal_print(o, static_cast<char>(value)); }

void dynd::hexadecimal_print(std::ostream &o, unsigned short value)
{
  // Standard printing is in big-endian order
  hexadecimal_print(o, static_cast<char>((value >> 8) & 0xff));
  hexadecimal_print(o, static_cast<char>(value & 0xff));
}

void dynd::hexadecimal_print(std::ostream &o, unsigned int value)
{
  // Standard printing is in big-endian order
  hexadecimal_print(o, static_cast<char>(value >> 24));
  hexadecimal_print(o, static_cast<char>((value >> 16) & 0xff));
  hexadecimal_print(o, static_cast<char>((value >> 8) & 0xff));
  hexadecimal_print(o, static_cast<char>(value & 0xff));
}

void dynd::hexadecimal_print(std::ostream &o, unsigned long value)
{
  if (sizeof(unsigned int) == sizeof(unsigned long)) {
    hexadecimal_print(o, static_cast<unsigned int>(value));
  }
  else {
    hexadecimal_print(o, static_cast<unsigned long long>(value));
  }
}

void dynd::hexadecimal_print(std::ostream &o, unsigned long long value)
{
  // Standard printing is in big-endian order
  hexadecimal_print(o, static_cast<char>(value >> 56));
  hexadecimal_print(o, static_cast<char>((value >> 48) & 0xff));
  hexadecimal_print(o, static_cast<char>((value >> 40) & 0xff));
  hexadecimal_print(o, static_cast<char>((value >> 32) & 0xff));
  hexadecimal_print(o, static_cast<char>((value >> 24) & 0xff));
  hexadecimal_print(o, static_cast<char>((value >> 16) & 0xff));
  hexadecimal_print(o, static_cast<char>((value >> 8) & 0xff));
  hexadecimal_print(o, static_cast<char>(value & 0xff));
}

void dynd::hexadecimal_print(std::ostream &o, const char *data, intptr_t element_size)
{
  for (int i = 0; i < element_size; ++i, ++data) {
    hexadecimal_print(o, *data);
  }
}

void dynd::hexadecimal_print_summarized(std::ostream &o, const char *data, intptr_t element_size, intptr_t summary_size)
{
  if (element_size * 2 <= summary_size) {
    hexadecimal_print(o, data, element_size);
  }
  else {
    // Print a summary
    intptr_t size = max(summary_size / 4 - 1, (intptr_t)1);
    hexadecimal_print(o, data, size);
    o << " ... ";
    size = max(summary_size / 4 - size - 1, (intptr_t)1);
    hexadecimal_print(o, data + element_size - size, size);
  }
}

static intptr_t line_count(const std::string &s)
{
  return 1 + count_if(s.begin(), s.end(), bind1st(equal_to<char>(), '\n'));
}

static void summarize_stats(const std::string &s, intptr_t &num_rows, intptr_t &max_num_cols)
{
  num_rows += line_count(s);
  max_num_cols = max(max_num_cols, (intptr_t)s.size());
}

void dynd::print_indented(ostream &o, const std::string &indent, const std::string &s, bool skipfirstline)
{
  const char *begin = s.data();
  const char *end = s.data() + s.size();
  const char *cur = begin;
  while (cur != end) {
    const char *next = find_if(cur, end, bind1st(equal_to<char>(), '\n'));
    if (*next == '\n')
      ++next;
    if (!skipfirstline || cur != begin)
      o << indent;
    o.write(cur, next - cur);
    cur = next;
  }
}

// TODO Move the magic numbers into parameters
void dynd::strided_array_summarized(std::ostream &o, const ndt::type &tp, const char *arrmeta, const char *data,
                                    intptr_t dim_size, intptr_t stride)
{
  const int leading_count = 7, trailing_count = 3, row_threshold = 10, col_threshold = 30, packing_cols = 75;

  vector<std::string> leading, trailing;
  intptr_t ilead = 0, itrail = dim_size - 1;
  intptr_t num_rows = 0, max_num_cols = 0;
  // Get leading strings
  for (; ilead < leading_count && ilead < dim_size; ++ilead) {
    stringstream ss;
    tp.print_data(ss, arrmeta, data + ilead * stride);
    leading.push_back(ss.str());
    summarize_stats(leading.back(), num_rows, max_num_cols);
  }
  // Get trailing strings
  for (itrail = max(dim_size - trailing_count, ilead + 1); itrail < dim_size; ++itrail) {
    stringstream ss;
    tp.print_data(ss, arrmeta, data + itrail * stride);
    trailing.push_back(ss.str());
    summarize_stats(trailing.back(), num_rows, max_num_cols);
  }
  itrail = dim_size - trailing.size() - 1;

  // Select between two printing strategies depending on what we got
  if ((size_t)num_rows > (leading.size() + trailing.size()) || max_num_cols > col_threshold) {
    // Trim the leading/trailing vectors until we get to our threshold
    while (num_rows > row_threshold && (trailing.size() > 1 || leading.size() > 1)) {
      if (trailing.size() > 1) {
        num_rows -= line_count(trailing.front());
        trailing.erase(trailing.begin());
      }
      if (num_rows > row_threshold && leading.size() > 1) {
        if (trailing.empty()) {
          trailing.insert(trailing.begin(), leading.back());
        }
        else {
          num_rows -= line_count(leading.back());
        }
        leading.pop_back();
      }
    }
    // Print the [leading, ..., trailing]
    o << "[";
    print_indented(o, " ", leading.front(), true);
    for (size_t i = 1; i < leading.size(); ++i) {
      o << ",\n";
      print_indented(o, " ", leading[i]);
    }
    if (leading.size() != (size_t)dim_size) {
      if ((size_t)dim_size > (leading.size() + trailing.size())) {
        o << ",\n ...\n";
      }
      if (!trailing.empty()) {
        print_indented(o, " ", trailing.front());
      }
      for (size_t i = 1; i < trailing.size(); ++i) {
        o << ",\n";
        print_indented(o, " ", trailing[i]);
      }
    }
    o << "]";
  }
  else {
    // Pack the values in a regular grid
    // Keep getting more strings until we use up our column budget.
    intptr_t total_cols = (max_num_cols + 2) * (leading.size() + trailing.size());
    while (ilead <= itrail && total_cols < packing_cols * row_threshold) {
      if (ilead <= itrail) {
        stringstream ss;
        tp.print_data(ss, arrmeta, data + ilead++ * stride);
        leading.push_back(ss.str());
        summarize_stats(leading.back(), num_rows, max_num_cols);
      }
      if (ilead <= itrail) {
        stringstream ss;
        tp.print_data(ss, arrmeta, data + itrail-- * stride);
        trailing.insert(trailing.begin(), ss.str());
        summarize_stats(trailing.front(), num_rows, max_num_cols);
      }
      total_cols = (max_num_cols + 2) * (leading.size() + trailing.size());
    }

    intptr_t per_row = packing_cols / (max_num_cols + 2);

    if (leading.size() + trailing.size() == (size_t)dim_size) {
      // Combine the lists if the total size is covered
      copy(trailing.begin(), trailing.end(), back_inserter(leading));
      trailing.clear();
    }
    else {
      // Remove partial rows if the total size is not covered
      if (leading.size() > (size_t)per_row && leading.size() % per_row != 0) {
        leading.erase(leading.begin() + (leading.size() / per_row) * per_row, leading.end());
      }
      if (trailing.size() > (size_t)per_row && trailing.size() % per_row != 0) {
        trailing.erase(trailing.begin(), trailing.begin() + trailing.size() % per_row);
      }
    }

    intptr_t i = 0, j;
    intptr_t i_size = leading.size();
    if (!i_size)
      o << '[';
    while (i < i_size) {
      o << ((i == 0) ? "[" : " ") << setw(max_num_cols) << leading[i] << setw(0);
      ++i;
      for (j = 1; j < per_row && i < i_size; ++j, ++i) {
        o << ", " << setw(max_num_cols) << leading[i] << setw(0);
      }
      if (i < i_size - 1) {
        o << ",\n";
      }
    }
    if (leading.size() != (size_t)dim_size) {
      i = 0;
      i_size = trailing.size();
      o << ",\n ...\n";
      while (i < i_size) {
        o << " " << setw(max_num_cols) << trailing[i] << setw(0);
        ++i;
        for (j = 1; j < per_row && i < i_size; ++j, ++i) {
          o << ", " << setw(max_num_cols) << trailing[i] << setw(0);
        }
        if (i < i_size - 1) {
          o << ",\n";
        }
      }
    }
    o << "]";
  }
}

void dynd::print_builtin_scalar(type_id_t type_id, std::ostream &o, const char *data)
{
  switch (type_id) {
  case bool_id:
    o << (*data ? "True" : "False");
    break;
  case int8_id:
    print_as<int8, int32>(o, data);
    break;
  case int16_id:
    print_as<int16, int32>(o, data);
    break;
  case int32_id:
    print_as<int32, int32>(o, data);
    break;
  case int64_id:
    print_as<int64, int64>(o, data);
    break;
  case int128_id:
    print_as<int128, int128>(o, data);
    break;
  case uint8_id:
    print_as<uint8, uint32>(o, data);
    break;
  case uint16_id:
    print_as<uint16, uint32>(o, data);
    break;
  case uint32_id:
    print_as<uint32, uint32>(o, data);
    break;
  case uint64_id:
    print_as<uint64, uint64>(o, data);
    break;
  case uint128_id:
    print_as<uint128, uint128>(o, data);
    break;
  case float16_id:
    print_as<float16, float32>(o, data);
    break;
  case float32_id:
    print_as<float32, float32>(o, data);
    break;
  case float64_id:
    print_as<float64, float64>(o, data);
    break;
  case float128_id:
    print_as<float128, float128>(o, data);
    break;
  case complex_float32_id:
    print_as<complex<float>, complex<float>>(o, data);
    break;
  case complex_float64_id:
    print_as<complex<double>, complex<double>>(o, data);
    break;
  case void_id:
    o << "(void)";
    break;
  default:
    stringstream ss;
    ss << "printing of dynd builtin type id " << type_id << " isn't supported yet";
    throw dynd::type_error(ss.str());
  }
}

void ndt::type::print_data(std::ostream &o, const char *arrmeta, const char *data) const
{
  if (is_builtin()) {
    print_builtin_scalar(get_id(), o, data);
  }
  else {
    extended()->print_data(o, arrmeta, data);
  }
}

type_id_t ndt::type::get_base_id() const { return type_registry[get_id()].bases[0]; }

std::map<std::array<type_id_t, 2>, ndt::common_type::child_type> ndt::common_type::children;

struct ndt::common_type::init {
  template <typename TypeIDSequence>
  void on_each()
  {
    children[{{front<TypeIDSequence>::value, back<TypeIDSequence>::value}}] =
        [](const ndt::type &DYND_UNUSED(tp0), const ndt::type &DYND_UNUSED(tp1)) {
          return ndt::make_type<
              typename std::common_type<typename dynd::type_of<front<TypeIDSequence>::value>::type,
                                        typename dynd::type_of<back<TypeIDSequence>::value>::type>::type>();
        };
  }
};

ndt::common_type::common_type()
{
  typedef type_id_sequence<int32_id, float64_id, int64_id, float32_id> I;
  for_each<typename outer<I, I>::type>(init());

  typedef type_id_sequence<int32_id, float64_id, int64_id, float32_id, fixed_dim_id> J;

  for (type_id_t tp_id : i2a<J>()) {
    children[{{option_id, tp_id}}] = [](const ndt::type &tp0, const ndt::type &tp1) {
      return make_type<option_type>(ndt::common_type(tp0.extended<option_type>()->get_value_type(), tp1));
    };
    children[{{tp_id, option_id}}] = [](const ndt::type &tp0, const ndt::type &tp1) {
      return make_type<option_type>(ndt::common_type(tp0, tp1.extended<option_type>()->get_value_type()));
    };
    children[{{any_kind_id, tp_id}}] = [](const ndt::type &DYND_UNUSED(tp0), const ndt::type &tp1) { return tp1; };
    children[{{tp_id, any_kind_id}}] = [](const ndt::type &tp0, const ndt::type &DYND_UNUSED(tp1)) { return tp0; };
  }
  children[{{fixed_dim_id, fixed_dim_id}}] = [](const ndt::type &tp0, const ndt::type &tp1) {
    if (tp0.extended<fixed_dim_type>()->get_fixed_dim_size() != tp1.extended<fixed_dim_type>()->get_fixed_dim_size()) {
      return ndt::var_dim_type::make(ndt::common_type(tp0.extended<fixed_dim_type>()->get_element_type(),
                                                      tp1.extended<fixed_dim_type>()->get_element_type()));
    }
    return ndt::make_fixed_dim(tp0.extended<fixed_dim_type>()->get_fixed_dim_size(),
                               ndt::common_type(tp0.extended<fixed_dim_type>()->get_element_type(),
                                                tp1.extended<fixed_dim_type>()->get_element_type()));
  };
  children[{{fixed_dim_id, var_dim_id}}] = [](const ndt::type &tp0, const ndt::type &tp1) {
    return ndt::var_dim_type::make(ndt::common_type(tp0.extended<fixed_dim_type>()->get_element_type(),
                                                    tp1.extended<fixed_dim_type>()->get_element_type()));
  };
  children[{{var_dim_id, fixed_dim_id}}] = children[{{fixed_dim_id, var_dim_id}}];
}

ndt::type ndt::common_type::operator()(const ndt::type &tp0, const ndt::type &tp1) const
{
  child_type child = children[{{tp0.get_id(), tp1.get_id()}}];
  if (child == NULL) {
    return type();
  }

  return child(tp0, tp1);
}

class ndt::common_type ndt::common_type;

// Returns true if the destination type can represent *all* the values
// of the source type, false otherwise. This is used, for example,
// to skip any overflow checks when doing value assignments between differing
// types.
bool dynd::is_lossless_assignment(const ndt::type &dst_tp, const ndt::type &src_tp)
{
  if (dst_tp.is_builtin() && src_tp.is_builtin()) {
    switch (src_tp.get_kind()) {
    case kind_kind: // TODO: raise an error?
      return true;
    case pattern_kind: // TODO: raise an error?
      return true;
    case bool_kind:
      switch (dst_tp.get_kind()) {
      case bool_kind:
      case sint_kind:
      case uint_kind:
      case real_kind:
      case complex_kind:
        return true;
      case bytes_kind:
        return false;
      default:
        break;
      }
      break;
    case sint_kind:
      switch (dst_tp.get_kind()) {
      case bool_kind:
        return false;
      case sint_kind:
        return dst_tp.get_data_size() >= src_tp.get_data_size();
      case uint_kind:
        return false;
      case real_kind:
        return dst_tp.get_data_size() > src_tp.get_data_size();
      case complex_kind:
        return dst_tp.get_data_size() > 2 * src_tp.get_data_size();
      case bytes_kind:
        return false;
      default:
        break;
      }
      break;
    case uint_kind:
      switch (dst_tp.get_kind()) {
      case bool_kind:
        return false;
      case sint_kind:
        return dst_tp.get_data_size() > src_tp.get_data_size();
      case uint_kind:
        return dst_tp.get_data_size() >= src_tp.get_data_size();
      case real_kind:
        return dst_tp.get_data_size() > src_tp.get_data_size();
      case complex_kind:
        return dst_tp.get_data_size() > 2 * src_tp.get_data_size();
      case bytes_kind:
        return false;
      default:
        break;
      }
      break;
    case real_kind:
      switch (dst_tp.get_kind()) {
      case bool_kind:
      case sint_kind:
      case uint_kind:
        return false;
      case real_kind:
        return dst_tp.get_data_size() >= src_tp.get_data_size();
      case complex_kind:
        return dst_tp.get_data_size() >= 2 * src_tp.get_data_size();
      case bytes_kind:
        return false;
      default:
        break;
      }
    case complex_kind:
      switch (dst_tp.get_kind()) {
      case bool_kind:
      case sint_kind:
      case uint_kind:
      case real_kind:
        return false;
      case complex_kind:
        return dst_tp.get_data_size() >= src_tp.get_data_size();
      case bytes_kind:
        return false;
      default:
        break;
      }
    case string_kind:
      switch (dst_tp.get_kind()) {
      case bool_kind:
      case sint_kind:
      case uint_kind:
      case real_kind:
      case complex_kind:
        return false;
      case bytes_kind:
        return false;
      default:
        break;
      }
    case bytes_kind:
      return dst_tp.get_kind() == bytes_kind && dst_tp.get_data_size() == src_tp.get_data_size();
    default:
      break;
    }

    throw std::runtime_error("unhandled built-in case in is_lossless_assignmently");
  }

  // Use the available base_type to check the casting
  if (!dst_tp.is_builtin()) {
    // Call with dst_dt (the first parameter) first
    return dst_tp.extended()->is_lossless_assignment(dst_tp, src_tp);
  }
  else {
    // Fall back to src_dt if the dst's extended is NULL
    return src_tp.extended()->is_lossless_assignment(dst_tp, src_tp);
  }
}
