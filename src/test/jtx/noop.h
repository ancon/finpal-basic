//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_JTX_NOOP_H_INCLUDED
#define MTCHAIN_TEST_JTX_NOOP_H_INCLUDED

#include <test/jtx/flags.h>

namespace mtchain {
namespace test {
namespace jtx {

/** The null transaction. */
inline
Json::Value
noop (Account const& account)
{
    return fset(account, 0);
}

} // jtx
} // test
} //

#endif
