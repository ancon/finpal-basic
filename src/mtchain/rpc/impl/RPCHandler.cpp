//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/rpc/RPCHandler.h>
#include <mtchain/rpc/impl/Tuning.h>
#include <mtchain/rpc/impl/Handler.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/basics/contract.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>
#include <mtchain/core/JobQueue.h>
#include <mtchain/json/Object.h>
#include <mtchain/json/to_string.h>
#include <mtchain/net/InfoSub.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Role.h>
#include <mtchain/resource/Fees.h>

namespace mtchain {
namespace RPC {

namespace {

/**
   This code is called from both the HTTP RPC handler and Websockets.

   The form of the Json returned is somewhat different between the two services.

   HTML:
     Success:
        {
           "result" : {
              "ledger" : {
                 "accepted" : false,
                 "transaction_hash" : "..."
              },
              "ledger_index" : 10300865,
              "validated" : false,
              "status" : "success"  # Status is inside the result.
           }
        }

     Failure:
        {
           "result" : {
              "error" : "noNetwork",
              "error_code" : 16,
              "error_message" : "Not synced to MTChain network.",
              "request" : {
                 "command" : "ledger",
                 "ledger_index" : 10300865
              },
              "status" : "error"
           }
        }

   Websocket:
     Success:
        {
           "result" : {
              "ledger" : {
                 "accepted" : false,
                 "transaction_hash" : "..."
              },
              "ledger_index" : 10300865,
              "validated" : false
           }
           "type": "response",
           "status": "success",   # Status is OUTside the result!
           "id": "client's ID",   # Optional
           "warning": 3.14        # Optional
        }

     Failure:
        {
          "error" : "noNetwork",
          "error_code" : 16,
          "error_message" : "Not synced to MTChain network.",
          "request" : {
             "command" : "ledger",
             "ledger_index" : 10300865
          },
          "type": "response",
          "status" : "error",
          "id": "client's ID"   # Optional
        }

 */

error_code_i fillHandler (Context& context,
                          boost::optional<Handler const&>& result)
{
    if (! isUnlimited (context.role))
    {
        // VFALCO NOTE Should we also add up the jtRPC jobs?
        //
        int jc = context.app.getJobQueue ().getJobCountGE (jtCLIENT);
        if (jc > Tuning::maxJobQueueClients)
        {
            JLOG (context.j.debug()) << "Too busy for command: " << jc;
            return rpcTOO_BUSY;
        }
    }

    if (!context.params.isMember(jss::command) && !context.params.isMember(jss::method))
        return rpcCOMMAND_MISSING;
    if (context.params.isMember(jss::command) && context.params.isMember(jss::method))
    {
        if (context.params[jss::command].asString() !=
            context.params[jss::method].asString())
            return rpcUNKNOWN_COMMAND;
    }

    std::string strCommand  = context.params.isMember(jss::command) ?
                              context.params[jss::command].asString() :
                              context.params[jss::method].asString();

    JLOG (context.j.trace()) << "COMMAND:" << strCommand;
    JLOG (context.j.trace()) << "REQUEST:" << context.params;

    auto handler = getHandler(strCommand);

    if (!handler)
        return rpcUNKNOWN_COMMAND;

    if (handler->role_ == Role::ADMIN && context.role != Role::ADMIN)
        return rpcNO_PERMISSION;

    if ((handler->condition_ & NEEDS_NETWORK_CONNECTION) &&
        (context.netOps.getOperatingMode () < NetworkOPs::omSYNCING))
    {
        JLOG (context.j.info())
            << "Insufficient network mode for RPC: "
            << context.netOps.strOperatingMode ();

        return rpcNO_NETWORK;
    }

    if (!context.app.config().standalone() &&
        handler->condition_ & NEEDS_CURRENT_LEDGER)
    {
        if (context.ledgerMaster.getValidatedLedgerAge () >
            Tuning::maxValidatedLedgerAge)
        {
            return rpcNO_CURRENT;
        }

        auto const cID = context.ledgerMaster.getCurrentLedgerIndex ();
        auto const vID = context.ledgerMaster.getValidLedgerIndex ();

        if (cID + 10 < vID)
        {
            JLOG (context.j.debug()) << "Current ledger ID(" << cID <<
                ") is less than validated ledger ID(" << vID << ")";
            return rpcNO_CURRENT;
        }
    }

    if ((handler->condition_ & NEEDS_CLOSED_LEDGER) &&
        !context.ledgerMaster.getClosedLedger ())
    {
        return rpcNO_CLOSED;
    }

    result = *handler;
    return rpcSUCCESS;
}

template <class Object, class Method>
Status callMethod (
    Context& context, Method method, std::string const& name, Object& result)
{
    try
    {
        auto v = context.app.getJobQueue().getLoadEventAP(
            jtGENERIC, "cmd:" + name);
        return method (context, result);
    }
    catch (std::exception& e)
    {
        JLOG (context.j.info()) << "Caught throw: " << e.what ();

        if (context.loadType == Resource::feeReferenceRPC)
            context.loadType = Resource::feeExceptionRPC;

        inject_error (rpcINTERNAL, result);
        return rpcINTERNAL;
    }
}

template <class Method, class Object>
void getResult (
    Context& context, Method method, Object& object, std::string const& name)
{
    auto&& result = Json::addObject (object, jss::result);
    if (auto status = callMethod (context, method, name, result))
    {
        JLOG (context.j.debug()) << "rpcError: " << status.toString();
        result[jss::status] = jss::error;
        result[jss::request] = context.params;
    }
    else
    {
        result[jss::status] = jss::success;
    }
}

} // namespace

Status doCommand (
    RPC::Context& context, Json::Value& result)
{
    boost::optional <Handler const&> handler;
    if (auto error = fillHandler (context, handler))
    {
        inject_error (error, result);
        return error;
    }

    if (auto method = handler->valueMethod_)
    {
        if (! context.headers.user.empty() ||
            ! context.headers.forwardedFor.empty())
        {
            JLOG(context.j.debug()) << "start command: " << handler->name_ <<
                ", X-User: " << context.headers.user << ", X-Forwarded-For: " <<
                    context.headers.forwardedFor;

            auto ret = callMethod (context, method, handler->name_, result);

            JLOG(context.j.debug()) << "finish command: " << handler->name_ <<
                ", X-User: " << context.headers.user << ", X-Forwarded-For: " <<
                    context.headers.forwardedFor;

            return ret;
        }
        else
        {
            return callMethod (context, method, handler->name_, result);
        }
    }

    return rpcUNKNOWN_COMMAND;
}

/** Execute an RPC command and store the results in a string. */
void executeRPC (
    RPC::Context& context, std::string& output)
{
    boost::optional <Handler const&> handler;
    if (auto error = fillHandler (context, handler))
    {
        auto wo = Json::stringWriterObject (output);
        auto&& sub = Json::addObject (*wo, jss::result);
        inject_error (error, sub);
    }
    else if (auto method = handler->objectMethod_)
    {
        auto wo = Json::stringWriterObject (output);
        getResult (context, method, *wo, handler->name_);
    }
    else if (auto method = handler->valueMethod_)
    {
        auto object = Json::Value (Json::objectValue);
        getResult (context, method, object, handler->name_);
        output = to_string (object);
    }
    else
    {
        // Can't ever get here.
        assert (false);
        Throw<std::logic_error> ("RPC handler with no method");
    }
}

Role roleRequired (std::string const& method)
{
    auto handler = RPC::getHandler(method);

    if (!handler)
        return Role::FORBID;

    return handler->role_;
}

} // RPC
} //
