//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/basics/make_lock.h>

namespace mtchain {

Json::Value doConsensusInfo (RPC::Context& context)
{
    Json::Value ret (Json::objectValue);

    {
        auto lock = make_lock(context.app.getMasterMutex());
        ret[jss::info] = context.netOps.getConsensusInfo ();
    }

    return ret;
}

} //
