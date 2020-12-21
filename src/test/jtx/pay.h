//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_JTX_PAY_H_INCLUDED
#define MTCHAIN_TEST_JTX_PAY_H_INCLUDED

#include <test/jtx/Account.h>
#include <test/jtx/amount.h>
#include <mtchain/json/json_value.h>

namespace mtchain {
namespace test {
namespace jtx {

/** Create a payment. */
Json::Value
pay (Account const& account,
    Account const& to, AnyAmount amount);

} // jtx
} // test
} //

#endif
