//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/json/json_writer.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/impl/Tuning.h>

#include <string>
#include <sstream>

namespace mtchain {

/** General RPC command that can retrieve objects in the account root.
    {
      account: <account>|<account_public_key>
      ledger_hash: <string> // optional
      ledger_index: <string | unsigned integer> // optional
      type: <string> // optional, defaults to all account objects types
      limit: <integer> // optional
      marker: <opaque> // optional, resume previous query
    }
*/

Json::Value doAccountObjects (RPC::Context& context)
{
    auto const& params = context.params;
    if (! params.isMember (jss::account))
        return RPC::missing_field_error (jss::account);

    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);
    if (ledger == nullptr)
        return result;

    AccountID accountID;
    {
        auto const strIdent = params[jss::account].asString ();
        if (auto jv = RPC::accountFromString (accountID, strIdent))
        {
            for (auto it = jv.begin (); it != jv.end (); ++it)
                result[it.memberName ()] = *it;

            return result;
        }
    }

    if (! ledger->exists(keylet::account (accountID)))
        return rpcError (rpcACT_NOT_FOUND);

    auto type = ltINVALID;
    if (params.isMember (jss::type))
    {
        static
        std::array<std::pair<char const *, LedgerEntryType>, 9> const
        types
        {{
            { jss::account, ltACCOUNT_ROOT },
            { jss::amendments, ltAMENDMENTS },
            { jss::directory, ltDIR_NODE },
            { jss::fee, ltFEE_SETTINGS },
            { jss::hashes, ltLEDGER_HASHES },
            { jss::offer, ltOFFER },
            { jss::signer_list, ltSIGNER_LIST },
            { jss::state, ltMTCHAIN_STATE },
            { jss::ticket, ltTICKET }
        }};

        auto const& p = params[jss::type];
        if (! p.isString ())
            return RPC::expected_field_error (jss::type, "string");

        auto const filter = p.asString ();
        auto iter = std::find_if (types.begin (), types.end (),
            [&filter](decltype (types.front ())& t)
                { return t.first == filter; });
        if (iter == types.end ())
            return RPC::invalid_field_error (jss::type);

        type = iter->second;
    }

    unsigned int limit;
    if (auto err = readLimitField(limit, RPC::Tuning::accountObjects, context))
        return *err;

    uint256 dirIndex;
    uint256 entryIndex;
    if (params.isMember (jss::marker))
    {
        auto const& marker = params[jss::marker];
        if (! marker.isString ())
            return RPC::expected_field_error (jss::marker, "string");

        std::stringstream ss (marker.asString ());
        std::string s;
        if (!std::getline(ss, s, ','))
            return RPC::invalid_field_error (jss::marker);

        if (! dirIndex.SetHex (s))
            return RPC::invalid_field_error (jss::marker);

        if (! std::getline (ss, s, ','))
            return RPC::invalid_field_error (jss::marker);

        if (! entryIndex.SetHex (s))
            return RPC::invalid_field_error (jss::marker);
    }

    if (! RPC::getAccountObjects (*ledger, accountID, type,
        dirIndex, entryIndex, limit, result))
    {
        result[jss::account_objects] = Json::arrayValue;
    }

    result[jss::account] = context.app.accountIDCache().toBase58 (accountID);
    context.loadType = Resource::feeMediumBurdenRPC;
    return result;
}

} //
