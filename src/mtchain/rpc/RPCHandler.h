//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_RPCHANDLER_H_INCLUDED
#define MTCHAIN_RPC_RPCHANDLER_H_INCLUDED

#include <mtchain/core/Config.h>
#include <mtchain/net/InfoSub.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/Status.h>

namespace mtchain {
namespace RPC {

struct Context;

/** Execute an RPC command and store the results in a Json::Value. */
Status doCommand (RPC::Context&, Json::Value&);

/** Execute an RPC command and store the results in an std::string. */
void executeRPC (RPC::Context&, std::string&);

Role roleRequired (std::string const& method );

} // RPC
} //

#endif
