//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/basics/Log.h>
#include <mtchain/json/json_value.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <boost/algorithm/string/predicate.hpp>

namespace mtchain {

Json::Value doLogLevel (RPC::Context& context)
{
    // log_level
    if (!context.params.isMember (jss::severity))
    {
        // get log severities
        Json::Value ret (Json::objectValue);
        Json::Value lev (Json::objectValue);

        lev[jss::base] =
            Logs::toString(Logs::fromSeverity(context.app.logs().threshold()));
        std::vector< std::pair<std::string, std::string> > logTable (
            context.app.logs().partition_severities());
        using stringPair = std::map<std::string, std::string>::value_type;
        for (auto const& it : logTable)
            lev[it.first] = it.second;

        ret[jss::levels] = lev;
        return ret;
    }

    LogSeverity const sv (
        Logs::fromString (context.params[jss::severity].asString ()));

    if (sv == lsINVALID)
        return rpcError (rpcINVALID_PARAMS);

    auto severity = Logs::toSeverity(sv);
    // log_level severity
    if (!context.params.isMember (jss::partition))
    {
        // set base log threshold
        context.app.logs().threshold(severity);
        return Json::objectValue;
    }

    // log_level partition severity base?
    if (context.params.isMember (jss::partition))
    {
        // set partition threshold
        std::string partition (context.params[jss::partition].asString ());

        if (boost::iequals (partition, "base"))
            context.app.logs().threshold (severity);
        else
            context.app.logs().get(partition).threshold(severity);

        return Json::objectValue;
    }

    return rpcError (rpcINVALID_PARAMS);
}

} //
