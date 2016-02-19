//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <dynd/types/categorical_kind_type.hpp>
#include <dynd/shape_tools.hpp>
#include <dynd/exceptions.hpp>

using namespace std;
using namespace dynd;

ndt::categorical_kind_type::categorical_kind_type()
    : base_type(categorical_id, kind_kind, 0, 0, type_flag_symbolic, 0, 0, 0)
{
}

ndt::categorical_kind_type::~categorical_kind_type() {}

size_t ndt::categorical_kind_type::get_default_data_size() const
{
  stringstream ss;
  ss << "Cannot get default data size of type " << type(this, true);
  throw runtime_error(ss.str());
}

void ndt::categorical_kind_type::print_data(std::ostream &DYND_UNUSED(o), const char *DYND_UNUSED(arrmeta),
                                            const char *DYND_UNUSED(data)) const
{
  throw type_error("Cannot store data of symbolic categorical_kind type");
}

void ndt::categorical_kind_type::print_type(std::ostream &o) const { o << "Categorical"; }

bool ndt::categorical_kind_type::is_expression() const { return false; }

bool ndt::categorical_kind_type::is_unique_data_owner(const char *DYND_UNUSED(arrmeta)) const { return false; }

ndt::type ndt::categorical_kind_type::get_canonical_type() const { return type(this, true); }

bool ndt::categorical_kind_type::is_lossless_assignment(const type &DYND_UNUSED(dst_tp),
                                                        const type &DYND_UNUSED(src_tp)) const
{
  return false;
}

bool ndt::categorical_kind_type::operator==(const base_type &rhs) const
{
  if (this == &rhs) {
    return true;
  }
  else {
    return rhs.get_kind() == kind_kind && rhs.get_id() == categorical_id;
  }
}

void ndt::categorical_kind_type::arrmeta_default_construct(char *DYND_UNUSED(arrmeta),
                                                           bool DYND_UNUSED(blockref_alloc)) const
{
  stringstream ss;
  ss << "Cannot default construct arrmeta for symbolic type " << type(this, true);
  throw runtime_error(ss.str());
}

void ndt::categorical_kind_type::arrmeta_copy_construct(
    char *DYND_UNUSED(dst_arrmeta), const char *DYND_UNUSED(src_arrmeta),
    const intrusive_ptr<memory_block_data> &DYND_UNUSED(embedded_reference)) const
{
  stringstream ss;
  ss << "Cannot copy construct arrmeta for symbolic type " << type(this, true);
  throw runtime_error(ss.str());
}

void ndt::categorical_kind_type::arrmeta_reset_buffers(char *DYND_UNUSED(arrmeta)) const {}

void ndt::categorical_kind_type::arrmeta_finalize_buffers(char *DYND_UNUSED(arrmeta)) const {}

void ndt::categorical_kind_type::arrmeta_destruct(char *DYND_UNUSED(arrmeta)) const {}

void ndt::categorical_kind_type::arrmeta_debug_print(const char *DYND_UNUSED(arrmeta), std::ostream &DYND_UNUSED(o),
                                                     const std::string &DYND_UNUSED(indent)) const
{
  stringstream ss;
  ss << "Cannot have arrmeta for symbolic type " << type(this, true);
  throw runtime_error(ss.str());
}

void ndt::categorical_kind_type::data_destruct(const char *DYND_UNUSED(arrmeta), char *DYND_UNUSED(data)) const
{
  stringstream ss;
  ss << "Cannot have data for symbolic type " << type(this, true);
  throw runtime_error(ss.str());
}

void ndt::categorical_kind_type::data_destruct_strided(const char *DYND_UNUSED(arrmeta), char *DYND_UNUSED(data),
                                                       intptr_t DYND_UNUSED(stride), size_t DYND_UNUSED(count)) const
{
  stringstream ss;
  ss << "Cannot have data for symbolic type " << type(this, true);
  throw runtime_error(ss.str());
}

bool ndt::categorical_kind_type::match(const char *DYND_UNUSED(arrmeta), const type &candidate_tp,
                                       const char *DYND_UNUSED(candidate_arrmeta),
                                       std::map<std::string, type> &DYND_UNUSED(tp_vars)) const
{
  return candidate_tp.get_id() == categorical_id;
}
