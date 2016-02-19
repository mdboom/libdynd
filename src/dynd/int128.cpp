//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <stdexcept>
#include <sstream>
#include <cmath>

#include <dynd/config.hpp>

using namespace std;
using namespace dynd;

#if !defined(DYND_HAS_INT128)

dynd::int128::int128(float value)
{
  bool neg = (value < 0);
  if (value < 0) {
    value = -value;
  }
  if (value >= 18446744073709551616.0) {
    m_hi = (uint64_t)((double)value / 18446744073709551616.0);
    m_lo = (uint64_t)fmod((double)value, 18446744073709551616.0);
  } else {
    m_hi = 0;
    m_lo = (uint64_t)value;
  }
  if (neg) {
    negate();
  }
}

dynd::int128::int128(double value)
{
  bool neg = (value < 0);
  if (value < 0) {
    value = -value;
  }
  if (value >= 18446744073709551616.0) {
    m_hi = (uint64_t)(value / 18446744073709551616.0);
    m_lo = (uint64_t)fmod(value, 18446744073709551616.0);
  } else {
    m_hi = 0;
    m_lo = (uint64_t)value;
  }
  if (neg) {
    negate();
  }
}

#if defined(DYND_HAS_INT128)
dynd::int128::int128(const uint128 &value)
    : m_lo((uint64_t)value), m_hi((uint64_t)(value >> 64))
{
}
#else
dynd::int128::int128(const uint128 &value) : m_lo(value.m_lo), m_hi(value.m_hi)
{
}
#endif

dynd::int128::int128(const float16 &value)
    : m_lo((int64_t)value), m_hi(value.signbit_() ? 0xffffffffffffffffULL : 0UL)
{
}

dynd::int128::int128(const float128 &DYND_UNUSED(value))
{
#ifndef __CUDA_ARCH__
  throw runtime_error("dynd float128 to int128 conversion is not implemented");
#endif
}

int128 dynd::int128::operator*(uint32_t rhs) const
{
  if ((int64_t)m_hi < 0) {
    // TODO: fix case for minimum int, which will recurse forever
    return -(-(*this) * rhs);
  } else {
    // Split the multiplication into three
    // First product
    uint64_t lo_partial = (m_lo & 0x00000000ffffffffULL) * rhs;
    // Second product
    uint64_t tmp = ((m_lo & 0xffffffff00000000ULL) >> 32) * rhs;
    uint64_t lo = lo_partial + (tmp << 32);
    uint64_t hi = (tmp >> 32) + (lo < lo_partial);
    // Third product
    hi += m_hi * rhs;
    return int128(hi, lo);
  }
}

/*
int128 dynd::int128::operator/(uint32_t rhs) const
{
  if ((int64_t)m_hi < 0) {
    // TODO: fix case for minimum int, which will recurse forever
    return 0;
    //        return -(-(*this) / rhs);
  } else {
    // Split the division into three
    // First division (bits 127..64)
    uint64_t hi_div = m_hi / rhs;
    uint64_t hi_rem = m_hi % rhs;
    // Second division (bits 63..32)
    uint64_t mid_val = (hi_rem << 32) | (m_lo >> 32);
    uint64_t mid_div = mid_val / rhs;
    uint64_t mid_rem = mid_val % rhs;
    // Third division (bits 31..0)
    uint64_t low_val = (mid_rem << 32) | (m_lo & 0x00000000ffffffffULL);
    uint64_t low_div = low_val / rhs;
    return int128(hi_div, (mid_div << 32) | low_div);
  }
}
*/

std::ostream &dynd::operator<<(ostream &out, const int128 &val)
{
  if (val < int128(0)) {
    int128 tmp = -val;
    return (out << "-" << uint128(tmp.m_hi, tmp.m_lo));
  } else {
    return (out << uint128(val.m_hi, val.m_lo));
  }
}

#endif // !defined(DYND_HAS_INT128)
