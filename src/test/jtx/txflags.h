//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TEST_JTX_TXFLAGS_H_INCLUDED
#define MTCHAIN_TEST_JTX_TXFLAGS_H_INCLUDED

#include <test/jtx/Env.h>

namespace mtchain {
namespace test {
namespace jtx {

/** Set the flags on a JTx. */
class txflags
{
private:
    std::uint32_t v_;

public:
    explicit
    txflags (std::uint32_t v)
        : v_(v)
    {
    }

    void
    operator()(Env&, JTx& jt) const;
};

} // jtx
} // test
} //

#endif
