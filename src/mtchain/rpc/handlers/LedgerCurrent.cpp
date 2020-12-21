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
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

Json::Value doLedgerCurrent (RPC::Context& context)
{
    Json::Value jvResult;
    jvResult[jss::ledger_current_index] =
        context.ledgerMaster.getCurrentLedgerIndex ();
    return jvResult;
}

} //
