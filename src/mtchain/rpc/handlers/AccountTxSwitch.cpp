//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/json/json_value.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

Json::Value doAccountTxOld (RPC::Context& context);
        Json::Value doAccountTx (RPC::Context& context);

// Temporary switching code until the old account_tx is removed
Json::Value doAccountTxSwitch (RPC::Context& context)
{
    if (context.params.isMember(jss::offset) ||
        context.params.isMember(jss::count) ||
        context.params.isMember(jss::descending) ||
        context.params.isMember(jss::ledger_max) ||
        context.params.isMember(jss::ledger_min))
    {
        return doAccountTxOld(context);
    }
    return doAccountTx(context);
}

} //
