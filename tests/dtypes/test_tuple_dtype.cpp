//
// Copyright (C) 2011-13, DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <iostream>
#include <sstream>
#include <stdexcept>
#include "inc_gtest.hpp"

#include <dynd/ndobject.hpp>
#include <dynd/dtypes/tuple_dtype.hpp>
#include <dynd/dtypes/fixedstring_dtype.hpp>

using namespace std;
using namespace dynd;

TEST(TupleDType, CreateOneField) {
    dtype dt;
    const tuple_dtype *tdt;

    // Tuple with one field
    dt = make_tuple_dtype(make_dtype<int32_t>());
    EXPECT_EQ(tuple_type_id, dt.get_type_id());
    EXPECT_EQ(4u, dt.get_data_size());
    EXPECT_EQ(4u, dt.get_alignment());
    EXPECT_TRUE(dt.is_pod());
    EXPECT_EQ(0u, (dt.get_flags()&(dtype_flag_blockref|dtype_flag_destructor)));
    tdt = static_cast<const tuple_dtype *>(dt.extended());
    EXPECT_TRUE(tdt->is_standard_layout());
    EXPECT_EQ(1u, tdt->get_fields().size());
    EXPECT_EQ(1u, tdt->get_offsets().size());
    EXPECT_EQ(make_dtype<int32_t>(), tdt->get_fields()[0]);
    EXPECT_EQ(0u, tdt->get_offsets()[0]);
}

TEST(TupleDType, CreateTwoField) {
    dtype dt;
    const tuple_dtype *tdt;


    // Tuple with two fields
    dt = make_tuple_dtype(make_dtype<int64_t>(), make_dtype<int32_t>());
    EXPECT_EQ(tuple_type_id, dt.get_type_id());
    EXPECT_EQ(16u, dt.get_data_size());
    EXPECT_EQ(8u, dt.get_alignment());
    EXPECT_TRUE(dt.is_pod());
    EXPECT_EQ(0u, (dt.get_flags()&(dtype_flag_blockref|dtype_flag_destructor)));
    tdt = static_cast<const tuple_dtype *>(dt.extended());
    EXPECT_TRUE(tdt->is_standard_layout());
    EXPECT_EQ(2u, tdt->get_fields().size());
    EXPECT_EQ(2u, tdt->get_offsets().size());
    EXPECT_EQ(make_dtype<int64_t>(), tdt->get_fields()[0]);
    EXPECT_EQ(make_dtype<int32_t>(), tdt->get_fields()[1]);
    EXPECT_EQ(0u, tdt->get_offsets()[0]);
    EXPECT_EQ(8u, tdt->get_offsets()[1]);
}

TEST(TupleDType, CreateThreeField) {
    dtype dt;
    const tuple_dtype *tdt;

    // Tuple with three fields
    dtype d1 = make_dtype<int64_t>();
    dtype d2 = make_dtype<int32_t>();
    dtype d3 = make_fixedstring_dtype(5, string_encoding_utf_8);
    dt = make_tuple_dtype(d1, d2, d3);
    EXPECT_EQ(tuple_type_id, dt.get_type_id());
    EXPECT_EQ(24u, dt.get_data_size());
    EXPECT_EQ(8u, dt.get_alignment());
    EXPECT_TRUE(dt.is_pod());
    EXPECT_EQ(0u, (dt.get_flags()&(dtype_flag_blockref|dtype_flag_destructor)));
    tdt = static_cast<const tuple_dtype *>(dt.extended());
    EXPECT_TRUE(tdt->is_standard_layout());
    EXPECT_EQ(3u, tdt->get_fields().size());
    EXPECT_EQ(3u, tdt->get_offsets().size());
    EXPECT_EQ(make_dtype<int64_t>(), tdt->get_fields()[0]);
    EXPECT_EQ(make_dtype<int32_t>(), tdt->get_fields()[1]);
    EXPECT_EQ(make_fixedstring_dtype(5, string_encoding_utf_8), tdt->get_fields()[2]);
    EXPECT_EQ(0u, tdt->get_offsets()[0]);
    EXPECT_EQ(8u, tdt->get_offsets()[1]);
    EXPECT_EQ(12u, tdt->get_offsets()[2]);

}
