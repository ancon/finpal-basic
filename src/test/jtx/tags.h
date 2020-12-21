//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_JTX_TAGS_H_INCLUDED
#define MTCHAIN_TEST_JTX_TAGS_H_INCLUDED

namespace mtchain {
namespace test {

namespace jtx {

struct none_t { none_t() { } };
static none_t const none;

struct autofill_t { autofill_t() { } };
static autofill_t const autofill;

struct disabled_t { disabled_t() { } };
static disabled_t const disabled;

} // jtx

} // test
} //

#endif
