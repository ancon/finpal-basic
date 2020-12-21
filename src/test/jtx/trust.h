//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_JTX_TRUST_H_INCLUDED
#define MTCHAIN_TEST_JTX_TRUST_H_INCLUDED

#include <test/jtx/Account.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/STAmount.h>

namespace mtchain {
namespace test {
namespace jtx {

/** Modify a trust line. */
Json::Value
trust (Account const& account,
    STAmount const& amount,
       std::uint32_t flags=0);

/** Change flags on a trust line. */
Json::Value
trust (Account const& account,
    STAmount const& amount,
    Account const& peer,
    std::uint32_t flags);

} // jtx
} // test
} //

#endif
