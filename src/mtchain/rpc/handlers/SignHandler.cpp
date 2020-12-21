//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/TransactionSign.h>

namespace mtchain {

// {
//   tx_json: <object>,
//   secret: <secret>
// }
Json::Value doSign (RPC::Context& context)
{
    context.loadType = Resource::feeHighBurdenRPC;
    NetworkOPs::FailHard const failType =
        NetworkOPs::doFailHard (
            context.params.isMember (jss::fail_hard)
            && context.params[jss::fail_hard].asBool ());

    return RPC::transactionSign (
        context.params,
        failType,
        context.role,
        context.ledgerMaster.getValidatedLedgerAge(),
        context.app);
}

} //
