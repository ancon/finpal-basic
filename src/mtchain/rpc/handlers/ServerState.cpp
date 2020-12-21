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
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/TransactionSign.h>
#include <mtchain/rpc/Role.h>

namespace mtchain {

Json::Value doServerState (RPC::Context& context)
{
    Json::Value ret (Json::objectValue);

    ret[jss::state] = context.netOps.getServerInfo (
        false, context.role == Role::ADMIN);

    return ret;
}

} //
