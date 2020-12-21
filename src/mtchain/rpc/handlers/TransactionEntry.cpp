//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>

namespace mtchain {

// {
//   ledger_hash : <ledger>,
//   ledger_index : <ledger_index>
// }
//
// XXX In this case, not specify either ledger does not mean ledger current. It
// means any ledger.
Json::Value doTransactionEntry (RPC::Context& context)
{
    std::shared_ptr<ReadView const> lpLedger;
    Json::Value jvResult = RPC::lookupLedger (lpLedger, context);

    if(! lpLedger)
        return jvResult;

    if(! context.params.isMember (jss::tx_hash))
    {
        jvResult[jss::error] = "fieldNotFoundTransaction";
    }
    else if(jvResult.get(jss::ledger_hash, Json::nullValue).isNull())
    {
        // We don't work on ledger current.

        // XXX We don't support any transaction yet.
        jvResult[jss::error]   = "notYetImplemented";
    }
    else
    {
        uint256 uTransID;
        // XXX Relying on trusted WSS client. Would be better to have a strict
        // routine, returning success or failure.
        uTransID.SetHex (context.params[jss::tx_hash].asString ());

        auto tx = lpLedger->txRead (uTransID);
        if(! tx.first)
        {
            jvResult[jss::error]   = "transactionNotFound";
        }
        else
        {
            jvResult[jss::tx_json]     = tx.first->getJson (0);
            if (tx.second)
                jvResult[jss::metadata]    = tx.second->getJson (0);
            // 'accounts'
            // 'engine_...'
            // 'ledger_...'
        }
    }

    return jvResult;
}

} //
