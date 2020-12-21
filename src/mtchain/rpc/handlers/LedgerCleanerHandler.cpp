//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/json/json_value.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/Handler.h>

namespace mtchain {

Json::Value doLedgerCleaner (RPC::Context& context)
{
    context.app.getLedgerMaster().doLedgerCleaner (context.params);
    return RPC::makeObjectValue ("Cleaner configured");
}


} //
