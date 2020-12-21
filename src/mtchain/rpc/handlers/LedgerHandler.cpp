//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/rpc/handlers/LedgerHandler.h>
#include <mtchain/app/ledger/LedgerToJson.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/LoadFeeTrack.h>
#include <mtchain/json/Object.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/Role.h>

namespace mtchain {
namespace RPC {

LedgerHandler::LedgerHandler (Context& context) : context_ (context)
{
}

Status LedgerHandler::check()
{
    auto const& params = context_.params;
    bool needsLedger = params.isMember (jss::ledger) ||
            params.isMember (jss::ledger_hash) ||
            params.isMember (jss::ledger_index);
    if (!needsLedger)
        return Status::OK;

    if (auto s = lookupLedger (ledger_, context_, result_))
        return s;

    bool bFull = params[jss::full].asBool();
    bool bTransactions = params[jss::transactions].asBool();
    bool bAccounts = params[jss::accounts].asBool();
    bool bExpand = params[jss::expand].asBool();
    bool bBinary = params[jss::binary].asBool();
    bool const owner_funds = params[jss::owner_funds].asBool();

    options_ = (bFull ? LedgerFill::full : 0)
            | (bExpand ? LedgerFill::expand : 0)
            | (bTransactions ? LedgerFill::dumpTm : 0)
            | (bAccounts ? LedgerFill::dumpState : 0)
            | (bBinary ? LedgerFill::binary : 0)
            | (owner_funds ? LedgerFill::ownerFunds : 0);

    if (bFull || bAccounts)
    {
        // Until some sane way to get full ledgers has been implemented,
        // disallow retrieving all state nodes.
        if (! isUnlimited (context_.role))
            return rpcNO_PERMISSION;

        if (context_.app.getFeeTrack().isLoadedLocal() &&
            !isUnlimited (context_.role))
        {
            return rpcTOO_BUSY;
        }
        context_.loadType = bBinary ? Resource::feeMediumBurdenRPC :
            Resource::feeHighBurdenRPC;
    }

    return Status::OK;
}

} // RPC
} //
