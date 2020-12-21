//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_RPC_CONTEXT_H_INCLUDED
#define MTCHAIN_RPC_CONTEXT_H_INCLUDED

#include <mtchain/core/Config.h>
#include <mtchain/core/JobQueue.h>
#include <mtchain/net/InfoSub.h>
#include <mtchain/rpc/Role.h>
#include <mtchain/json/Output.h>
#include <mtchain/beast/utility/Journal.h>

namespace mtchain {

class Application;
class NetworkOPs;
class LedgerMaster;

namespace RPC {

/** The context of information needed to call an RPC. */
struct Context
{
    /**
     * Data passed in from HTTP headers.
     */
    struct Headers
    {
        std::string user;
        std::string forwardedFor;
    };

    beast::Journal j;
    Json::Value params;
    Application& app;
    Resource::Charge& loadType;
    NetworkOPs& netOps;
    LedgerMaster& ledgerMaster;
    Resource::Consumer& consumer;
    Role role;
    std::shared_ptr<JobQueue::Coro> coro;
    InfoSub::pointer infoSub;
    Headers headers;
    Json::Output *output;
    bool noReply;
    bool closeSess;
};

} // RPC
} //

#endif
