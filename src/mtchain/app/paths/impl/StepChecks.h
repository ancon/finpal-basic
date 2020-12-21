//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_IMPL_STEP_CHECKS_H_INCLUDED
#define MTCHAIN_APP_PATHS_IMPL_STEP_CHECKS_H_INCLUDED

#include <mtchain/basics/Log.h>
#include <mtchain/beast/utility/Journal.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/AccountID.h>
#include <mtchain/protocol/UintTypes.h>

namespace mtchain {

inline
TER
checkFreeze (
    ReadView const& view,
    AccountID const& src,
    AccountID const& dst,
    Currency const& currency)
{
    assert (src != dst);

    // check freeze
    if (auto sle = view.read (keylet::account (dst)))
    {
        if (sle->isFlag (lsfGlobalFreeze))
        {
            return terNO_LINE;
        }
    }

    if (auto sle = view.read (keylet::line (src, dst, currency)))
    {
        if (sle->isFlag ((dst > src) ? lsfHighFreeze : lsfLowFreeze))
        {
            return terNO_LINE;
        }
    }

    return tesSUCCESS;
}

inline
TER
checkNoMTChain (
    ReadView const& view,
    AccountID const& prev,
    AccountID const& cur,
    // This is the account whose constraints we are checking
    AccountID const& next,
    Currency const& currency,
    beast::Journal j)
{
    // fetch the mtchain lines into and out of this node
    auto sleIn = view.read (keylet::line (prev, cur, currency));
    auto sleOut = view.read (keylet::line (cur, next, currency));

    if (!sleIn || !sleOut)
        return terNO_LINE;

    if ((*sleIn)[sfFlags] &
            ((cur > prev) ? lsfHighNoMtchain : lsfLowNoMtchain) &&
        (*sleOut)[sfFlags] &
            ((cur > next) ? lsfHighNoMtchain : lsfLowNoMtchain))
    {
        JLOG (j.info()) << "Path violates noMTChain constraint between " << prev
                      << ", " << cur << " and " << next;

        return terNO_MTCHAIN;
    }
    return tesSUCCESS;
}

}

#endif
