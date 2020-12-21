//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>

namespace mtchain {

Json::Value doFetchInfo (RPC::Context& context)
{
    Json::Value ret (Json::objectValue);

    if (context.params.isMember(jss::clear) && context.params[jss::clear].asBool())
    {
        context.netOps.clearLedgerFetch();
        ret[jss::clear] = true;
    }

    ret[jss::info] = context.netOps.getLedgerFetchInfo();

    return ret;
}

} //
