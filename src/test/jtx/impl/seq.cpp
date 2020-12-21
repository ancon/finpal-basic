//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <test/jtx/seq.h>
#include <mtchain/protocol/JsonFields.h>

namespace mtchain {
namespace test {
namespace jtx {

void
seq::operator()(Env&, JTx& jt) const
{
    if (! manual_)
        return;
    jt.fill_seq = false;
    if (num_)
        jt[jss::Sequence] = *num_;
}

} // jtx
} // test
} //
