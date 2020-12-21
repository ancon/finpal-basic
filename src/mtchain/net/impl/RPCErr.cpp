//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>

namespace mtchain {

struct RPCErr;

// VFALCO NOTE Deprecated function
Json::Value rpcError (int iError, Json::Value jvResult)
{
    RPC::inject_error (iError, jvResult);
    return jvResult;
}

// VFALCO NOTE Deprecated function
bool isRpcError (Json::Value jvResult)
{
    return jvResult.isObject () && jvResult.isMember (jss::error);
}

} //
