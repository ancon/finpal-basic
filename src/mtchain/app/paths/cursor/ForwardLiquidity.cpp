//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/cursor/PathCursor.h>
#include <mtchain/basics/Log.h>
#include <tuple>

namespace mtchain {
namespace path {

TER PathCursor::forwardLiquidity () const
{
    if (node().isAccount())
        return forwardLiquidityForAccount ();

    // Otherwise, node is an offer.
    if (previousNode().account_ == zero)
        return tesSUCCESS;

    // Previous is an account node, resolve its deliver.
    STAmount saInAct;
    STAmount saInFees;

    auto resultCode = deliverNodeForward (
        previousNode().account_,
        previousNode().saFwdDeliver, // Previous is sending this much.
        saInAct,
        saInFees,
        false);

    assert (resultCode != tesSUCCESS ||
            previousNode().saFwdDeliver == saInAct + saInFees);
    return resultCode;
}

} // path
} //

// Original comments:

// Called to drive the from the first offer node in a chain.
//
// - Offer input is in issuer/limbo.
// - Current offers consumed.
//   - Current offer owners debited.
//   - Transfer fees credited to issuer.
//   - Payout to issuer or limbo.
// - Deliver is set without transfer fees.
