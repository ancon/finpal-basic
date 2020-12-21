//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerToJson.h>
#include <mtchain/basics/strHex.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/impl/RPCHelpers.h>

namespace mtchain {

// {
//   ledger_hash : <ledger>
//   ledger_index : <ledger_index>
// }
Json::Value doLedgerHeader (RPC::Context& context)
{
    std::shared_ptr<ReadView const> lpLedger;
    auto jvResult = RPC::lookupLedger (lpLedger, context);

    if (!lpLedger)
        return jvResult;

    Serializer  s;
    addRaw (lpLedger->info(), s);
    jvResult[jss::ledger_data] = strHex (s.peekData ());

    // This information isn't verified: they should only use it if they trust
    // us.
    addJson (jvResult, {*lpLedger, 0});

    return jvResult;
}


} //
