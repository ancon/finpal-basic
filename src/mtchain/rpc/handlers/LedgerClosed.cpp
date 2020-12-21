//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

Json::Value doLedgerClosed (RPC::Context& context)
{
    auto ledger = context.ledgerMaster.getClosedLedger ();
    assert (ledger);

    Json::Value jvResult;
    jvResult[jss::ledger_index] = ledger->info().seq;
    jvResult[jss::ledger_hash] = to_string (ledger->info().hash);

    return jvResult;
}

} //
