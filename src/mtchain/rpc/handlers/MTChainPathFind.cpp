//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/paths/PathRequests.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/LegacyPathFind.h>
#include <mtchain/rpc/impl/RPCHelpers.h>

namespace mtchain {

// This interface is deprecated.
Json::Value doMTChainPathFind (RPC::Context& context)
{
    if (context.app.config().PATH_SEARCH_MAX == 0)
        return rpcError (rpcNOT_SUPPORTED);

    context.loadType = Resource::feeHighBurdenRPC;

    std::shared_ptr <ReadView const> lpLedger;
    Json::Value jvResult;

    if (! context.app.config().standalone() &&
        ! context.params.isMember(jss::ledger) &&
        ! context.params.isMember(jss::ledger_index) &&
        ! context.params.isMember(jss::ledger_hash))
    {
        // No ledger specified, use pathfinding defaults
        // and dispatch to pathfinding engine
        if (context.app.getLedgerMaster().getValidatedLedgerAge() >
            RPC::Tuning::maxValidatedLedgerAge)
        {
            return rpcError (rpcNO_NETWORK);
        }

        PathRequest::pointer request;
        lpLedger = context.ledgerMaster.getClosedLedger();

        jvResult = context.app.getPathRequests().makeLegacyPathRequest (
            request, std::bind(&JobQueue::Coro::post, context.coro),
                context.consumer, lpLedger, context.params);
        if (request)
        {
            context.coro->yield();
            jvResult = request->doStatus (context.params);
        }

        return jvResult;
    }

    // The caller specified a ledger
    jvResult = RPC::lookupLedger (lpLedger, context);
    if (! lpLedger)
        return jvResult;

    RPC::LegacyPathFind lpf (isUnlimited (context.role), context.app);
    if (! lpf.isOk ())
        return rpcError (rpcTOO_BUSY);

    auto result = context.app.getPathRequests().doLegacyPathRequest (
        context.consumer, lpLedger, context.params);

    for (auto &fieldName : jvResult.getMemberNames ())
        result[fieldName] = std::move (jvResult[fieldName]);

    return result;
}

} //
