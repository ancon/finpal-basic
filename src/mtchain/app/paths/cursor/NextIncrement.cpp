//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/cursor/MTChainLiquidity.h>
#include <mtchain/basics/contract.h>
#include <mtchain/basics/Log.h>

namespace mtchain {
namespace path {

// Calculate the next increment of a path.
//
// The increment is what can satisfy a portion or all of the requested output at
// the best quality.
//
// <-- pathState.uQuality
//
// This is the wrapper that restores a checkpointed version of the ledger so we
// can write all over it without consequence.

void PathCursor::nextIncrement () const
{
    // The next state is what is available in preference order.
    // This is calculated when referenced accounts changed.

    auto status = liquidity();

    if (status == tesSUCCESS)
    {
        if (pathState_.isDry())
        {
            JLOG (j_.debug())
                << "nextIncrement: success on dry path:"
                << " outPass=" << pathState_.outPass()
                << " inPass=" << pathState_.inPass();
            Throw<std::runtime_error> ("Made no progress.");
        }

        // Calculate relative quality.
        pathState_.setQuality(getRate (
            pathState_.outPass(), pathState_.inPass()));
    }
    else
    {
        pathState_.setQuality(0);
    }
    pathState_.setStatus (status);
}

} // path
} //
