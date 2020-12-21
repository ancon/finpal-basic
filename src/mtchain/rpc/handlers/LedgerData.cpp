//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerToJson.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/impl/Tuning.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/Role.h>

namespace mtchain {

// Get state nodes from a ledger
//   Inputs:
//     limit:        integer, maximum number of entries
//     marker:       opaque, resume point
//     binary:       boolean, format
//   Outputs:
//     ledger_hash:  chosen ledger's hash
//     ledger_index: chosen ledger's index
//     state:        array of state nodes
//     marker:       resume point, if any
Json::Value doLedgerData (RPC::Context& context)
{
    std::shared_ptr<ReadView const> lpLedger;
    auto const& params = context.params;

    auto jvResult = RPC::lookupLedger(lpLedger, context);
    if (!lpLedger)
        return jvResult;

    bool const isMarker = params.isMember (jss::marker);
    ReadView::key_type key = ReadView::key_type();
    if (isMarker)
    {
        Json::Value const& jMarker = params[jss::marker];
        if (! (jMarker.isString () && key.SetHex (jMarker.asString ())))
            return RPC::expected_field_error (jss::marker, "valid");
    }

    bool const isBinary = params[jss::binary].asBool();

    int limit = -1;
    if (params.isMember (jss::limit))
    {
        Json::Value const& jLimit = params[jss::limit];
        if (!jLimit.isIntegral ())
            return RPC::expected_field_error (jss::limit, "integer");

        limit = jLimit.asInt ();
    }

    auto maxLimit = RPC::Tuning::pageLength(isBinary);
    if ((limit < 0) || ((limit > maxLimit) && (! isUnlimited (context.role))))
        limit = maxLimit;

    jvResult[jss::ledger_hash] = to_string (lpLedger->info().hash);
    jvResult[jss::ledger_index] = lpLedger->info().seq;

    if (! isMarker)
    {
        // Return base ledger data on first query
        jvResult[jss::ledger] = getJson (
            LedgerFill (*lpLedger, isBinary ?
                LedgerFill::Options::binary : 0));
    }

    Json::Value& nodes = jvResult[jss::state];

    auto e = lpLedger->sles.end();
    for (auto i = lpLedger->sles.upper_bound(key); i != e; ++i)
    {
        auto sle = lpLedger->read(keylet::unchecked((*i)->key()));
        if (limit-- <= 0)
        {
            // Stop processing before the current key.
            auto k = sle->key();
            jvResult[jss::marker] = to_string(--k);
            break;
        }

        if (isBinary)
        {
            Json::Value& entry = nodes.append (Json::objectValue);
            entry[jss::data] = serializeHex(*sle);
            entry[jss::index] = to_string(sle->key());
        }
        else
        {
            Json::Value& entry = nodes.append (sle->getJson (0));
            entry[jss::index] = to_string(sle->key());
        }
    }

    return jvResult;
}

} //
