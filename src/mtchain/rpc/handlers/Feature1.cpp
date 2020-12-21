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
#include <mtchain/app/misc/AmendmentTable.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/beast/core/LexicalCast.h>

namespace mtchain {


// {
//   feature : <feature>
//   vetoed : true/false
// }
Json::Value doFeature (RPC::Context& context)
{

    // Get majority amendment status
    majorityAmendments_t majorities;

    if (auto const valLedger = context.ledgerMaster.getValidatedLedger())
        majorities = getMajorityAmendments (*valLedger);

    auto& table = context.app.getAmendmentTable ();

    if (!context.params.isMember (jss::feature))
    {
        auto features = table.getJson(0);

        for (auto const& m : majorities)
        {
            features[to_string(m.first)][jss::majority] =
                m.second.time_since_epoch().count();
        }

        Json::Value jvReply = Json::objectValue;
        jvReply[jss::features] = features;
        return jvReply;
    }

    auto feature = table.find (
        context.params[jss::feature].asString());

    if (!feature &&
        !feature.SetHexExact (context.params[jss::feature].asString ()))
        return rpcError (rpcBAD_FEATURE);

    if (context.params.isMember (jss::vetoed))
    {
        if (context.params[jss::vetoed].asBool ())
            context.app.getAmendmentTable().veto (feature);
        else
            context.app.getAmendmentTable().unVeto(feature);
    }

    Json::Value jvReply = table.getJson(feature);

    auto m = majorities.find (feature);
    if (m != majorities.end())
        jvReply [jss::majority] =
            m->second.time_since_epoch().count();

    return jvReply;
}


} //
