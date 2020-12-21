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
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/app/misc/Transaction.h>
#include <mtchain/json/json_value.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/types.h>
#include <mtchain/resource/Fees.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/Role.h>

namespace mtchain {

// {
//   account: account,
//   ledger_index_min: ledger_index  // optional, defaults to earliest
//   ledger_index_max: ledger_index, // optional, defaults to latest
//   binary: boolean,                // optional, defaults to false
//   forward: boolean,               // optional, defaults to false
//   limit: integer,                 // optional
//   marker: opaque                  // optional, resume previous query
// }
Json::Value doAccountTx (RPC::Context& context)
{
    auto& params = context.params;

    int limit = params.isMember (jss::limit) ?
            params[jss::limit].asUInt () : -1;
    bool bBinary = params.isMember (jss::binary) && params[jss::binary].asBool ();
    bool bForward = params.isMember (jss::forward) && params[jss::forward].asBool ();
    std::uint32_t   uLedgerMin;
    std::uint32_t   uLedgerMax;
    std::uint32_t   uValidatedMin;
    std::uint32_t   uValidatedMax;
    bool bValidated = context.ledgerMaster.getValidatedRange (uValidatedMin, uValidatedMax);

    if (!bValidated)
    {
        // Don't have a validated ledger range.
        return rpcError (rpcLGR_IDXS_INVALID);
    }

    if (!params.isMember (jss::account))
        return rpcError (rpcINVALID_PARAMS);

    auto const account = parseBase58<AccountID>(
        params[jss::account].asString());
    if (! account)
        return rpcError (rpcACT_MALFORMED);

    context.loadType = Resource::feeMediumBurdenRPC;

    if (params.isMember (jss::ledger_index_min) ||
        params.isMember (jss::ledger_index_max))
    {
        std::int64_t iLedgerMin  = params.isMember (jss::ledger_index_min)
                ? params[jss::ledger_index_min].asInt () : -1;
        std::int64_t iLedgerMax  = params.isMember (jss::ledger_index_max)
                ? params[jss::ledger_index_max].asInt () : -1;

        uLedgerMin  = iLedgerMin == -1 ? uValidatedMin :
            ((iLedgerMin >= uValidatedMin) ? iLedgerMin : uValidatedMin);
        uLedgerMax  = iLedgerMax == -1 ? uValidatedMax :
            ((iLedgerMax <= uValidatedMax) ? iLedgerMax : uValidatedMax);

        if (uLedgerMax < uLedgerMin)
            return rpcError (rpcLGR_IDXS_INVALID);
    }
    else
    {
        std::shared_ptr<ReadView const> ledger;
        auto ret = RPC::lookupLedger (ledger, context);

        if (! ledger)
            return ret;

	JLOG(context.j.warn()) << ledger->info().seq << " " << ret[jss::validated].asBool();
        if (! ret[jss::validated].asBool() || !context.ledgerMaster.haveLedger(ledger->info().seq)           /*(ledger->info().seq > uValidatedMax) || (ledger->info().seq < uValidatedMin)*/)
        {
            return rpcError (rpcLGR_NOT_VALIDATED);
        }

        uLedgerMin = uLedgerMax = ledger->info().seq;
    }

    Json::Value resumeToken;

    if (params.isMember(jss::marker))
         resumeToken = params[jss::marker];

#ifndef BEAST_DEBUG

    try
    {
#endif
        Json::Value ret (Json::objectValue);

        ret[jss::account] = context.app.accountIDCache().toBase58(*account);
        Json::Value& jvTxns = (ret[jss::transactions] = Json::arrayValue);

        if (bBinary)
        {
            auto txns = context.netOps.getTxsAccountB (
                *account, uLedgerMin, uLedgerMax, bForward, resumeToken, limit,
                isUnlimited (context.role));

            for (auto& it: txns)
            {
                Json::Value& jvObj = jvTxns.append (Json::objectValue);

                jvObj[jss::tx_blob] = std::get<0> (it);
                jvObj[jss::meta] = std::get<1> (it);

                std::uint32_t uLedgerIndex = std::get<2> (it);

                jvObj[jss::ledger_index] = uLedgerIndex;
                jvObj[jss::validated] = bValidated &&
		  /*uValidatedMin <= uLedgerIndex &&
		    uValidatedMax >= uLedgerIndex;*/
		  context.ledgerMaster.haveLedger(uLedgerIndex);
            }
        }
        else
        {
            auto txns = context.netOps.getTxsAccount (
                *account, uLedgerMin, uLedgerMax, bForward, resumeToken, limit,
                isUnlimited (context.role));

            for (auto& it: txns)
            {
                Json::Value& jvObj = jvTxns.append (Json::objectValue);

                if (it.first)
                    jvObj[jss::tx] = it.first->getJson (1);

                if (it.second)
                {
                    auto meta = it.second->getJson (1);
                    addPaymentDeliveredAmount (meta, context, it.first, it.second);
                    jvObj[jss::meta] = std::move(meta);

                    std::uint32_t uLedgerIndex = it.second->getLgrSeq ();

                    jvObj[jss::validated] = bValidated &&
		      /*uValidatedMin <= uLedgerIndex &&
                        uValidatedMax >= uLedgerIndex;*/
		      context.ledgerMaster.haveLedger(uLedgerIndex);
                }

            }
        }

        //Add information about the original query
        ret[jss::ledger_index_min] = uLedgerMin;
        ret[jss::ledger_index_max] = uLedgerMax;
        if (params.isMember (jss::limit))
            ret[jss::limit]        = limit;
        if (resumeToken)
            ret[jss::marker] = resumeToken;

        return ret;
#ifndef BEAST_DEBUG
    }
    catch (std::exception const&)
    {
        return rpcError (rpcINTERNAL);
    }

#endif
}

} //
