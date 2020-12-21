//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/cursor/MTChainLiquidity.h>
#include <mtchain/basics/Log.h>
#include <tuple>

namespace mtchain {
namespace path {

TER PathCursor::liquidity () const
{
    TER resultCode = tecPATH_DRY;
    PathCursor pc = *this;

    pathState_.resetView (mtchainCalc_.view);

    for (pc.nodeIndex_ = pc.nodeSize(); pc.nodeIndex_--; )
    {
        JLOG (j_.trace())
            << "reverseLiquidity>"
            << " nodeIndex=" << pc.nodeIndex_
            << ".issue_.account=" << to_string (pc.node().issue_.account);

        resultCode = pc.reverseLiquidity();

        if (!pc.node().transferRate_)
            return tefINTERNAL;

        JLOG (j_.trace())
            << "reverseLiquidity< "
            << "nodeIndex=" << pc.nodeIndex_
            << " resultCode=" << transToken (resultCode)
            << " transferRate_=" << *pc.node().transferRate_
            << ": " << resultCode;

        if (resultCode != tesSUCCESS)
            break;
    }

    // VFALCO-FIXME this generates errors
    // JLOG (j_.trace())
    //     << "nextIncrement: Path after reverse: " << pathState_.getJson ();

    if (resultCode != tesSUCCESS)
        return resultCode;

    pathState_.resetView (mtchainCalc_.view);

    for (pc.nodeIndex_ = 0; pc.nodeIndex_ < pc.nodeSize(); ++pc.nodeIndex_)
    {
        JLOG (j_.trace())
            << "forwardLiquidity> nodeIndex=" << nodeIndex_;

        resultCode = pc.forwardLiquidity();
        if (resultCode != tesSUCCESS)
            return resultCode;

        JLOG (j_.trace())
            << "forwardLiquidity<"
            << " nodeIndex:" << pc.nodeIndex_
            << " resultCode:" << resultCode;

        if (pathState_.isDry())
            resultCode = tecPATH_DRY;
    }
    return resultCode;
}

} // path
} //
