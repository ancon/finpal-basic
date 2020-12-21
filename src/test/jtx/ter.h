//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_JTX_TER_H_INCLUDED
#define MTCHAIN_TEST_JTX_TER_H_INCLUDED

#include <test/jtx/Env.h>
#include <tuple>

namespace mtchain {
namespace test {
namespace jtx {

/** Set the expected result code for a JTx
    The test will fail if the code doesn't match.
*/
class ter
{
private:
    boost::optional<TER> v_;

public:
    explicit
    ter (decltype(std::ignore))
    {
    }

    explicit
    ter (TER v)
        : v_(v)
    {
    }

    void
    operator()(Env&, JTx& jt) const
    {
        jt.ter = v_;
    }
};

} // jtx
} // test
} //

#endif
