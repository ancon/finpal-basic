//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>

#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/TxQ.h>
#include <mtchain/json/json_value.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/types.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>

namespace mtchain {

// {
//   account: <ident>,
//   strict: <bool>        // optional (default false)
//                         //   if true only allow public keys and addresses.
//   ledger_hash : <ledger>
//   ledger_index : <ledger_index>
//   signer_lists : <bool> // optional (default false)
//                         //   if true return SignerList(s).
//   queue : <bool>        // optional (default false)
//                         //   if true return information about transactions
//                         //   in the current TxQ, only if the requested
//                         //   ledger is open. Otherwise if true, returns an
//                         //   error.
// }

// TODO(tom): what is that "default"?
Json::Value doAccountInfo (RPC::Context& context)
{
    auto& params = context.params;

    std::string strIdent;
    if (params.isMember(jss::account))
        strIdent = params[jss::account].asString();
    else if (params.isMember(jss::ident))
        strIdent = params[jss::ident].asString();
    else
        return RPC::missing_field_error (jss::account);

    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);

    if (!ledger)
        return result;

    bool bStrict = params.isMember (jss::strict) && params[jss::strict].asBool ();
    AccountID accountID;

    // Get info on account.

    auto jvAccepted = RPC::accountFromString (accountID, strIdent, bStrict);

    if (jvAccepted)
        return jvAccepted;

    auto const sleAccepted = ledger->read(keylet::account(accountID));
    if (sleAccepted)
    {
        auto const queue = params.isMember(jss::queue) &&
            params[jss::queue].asBool();

        if (queue && !ledger->open())
        {
            // It doesn't make sense to request the queue
            // with any closed or validated ledger.
            RPC::inject_error(rpcINVALID_PARAMS, result);
            return result;
        }

        RPC::injectSLE(jvAccepted, *sleAccepted);
        result[jss::account_data] = jvAccepted;
        if (sleAccepted->isFieldPresent(sfIssues))
        {
            Json::Value jvTransferRates = Json::arrayValue;
            auto const& rates = sleAccepted->getFieldV256(sfIssues);
            for (auto const& k : rates)
            {
                auto const sle = ledger->read({ltMTCHAIN_STATE, k});
                if (!sle) continue;

                if (!sle->isFieldPresent(sfTransferRate))
                    continue;

                auto const& amount = sle->getFieldAmount(sfTransferRate);
                JLOG (context.j.warn()) << "transferRate = " << amount.getFullText();
                jvTransferRates.append(amount.getJson(2));
            }

            if (jvTransferRates.size () > 0)
            {
                result[jss::account_data][jss::TransferRates] = std::move(jvTransferRates);
            }
        }

        if (sleAccepted->isFieldPresent(sfIssues))
        {
            Json::Value jvLimits = Json::arrayValue;
            auto const& limits = sleAccepted->getFieldV256(sfIssues);
            for (auto const& k : limits)
            {
                auto const sle = ledger->read({ltMTCHAIN_STATE, k});
                if (!sle) continue;

                if (!sle->isFieldPresent(sfLimitAmount))
                    continue;

                auto const& limit = sle->getFieldAmount(sfLimitAmount);
                jvLimits.append(limit.getJson(2));
            }

            if (jvLimits.size () > 0)
            {
                result[jss::account_data][jss::LimitAmounts] = std::move(jvLimits);
            }
        }

        // Return SignerList(s) if that is requested.
        if (params.isMember (jss::signer_lists) &&
            params[jss::signer_lists].asBool ())
        {
            // We put the SignerList in an array because of an anticipated
            // future when we support multiple signer lists on one account.
            Json::Value jvSignerList = Json::arrayValue;

            // This code will need to be revisited if in the future we support
            // multiple SignerLists on one account.
            auto const sleSigners = ledger->read (keylet::signers (accountID));
            if (sleSigners)
                jvSignerList.append (sleSigners->getJson (0));

            result[jss::account_data][jss::signer_lists] =
                std::move(jvSignerList);
        }
        // Return queue info if that is requested
        if (queue)
        {
            Json::Value jvQueueData = Json::objectValue;

            auto const txs = context.app.getTxQ().getAccountTxs(
                accountID, *ledger);
            if (txs && !txs->empty())
            {
                jvQueueData[jss::txn_count] = static_cast<Json::UInt>(txs->size());
                jvQueueData[jss::lowest_sequence] = txs->begin()->first;
                jvQueueData[jss::highest_sequence] = txs->rbegin()->first;

                auto& jvQueueTx = jvQueueData[jss::transactions];
                jvQueueTx = Json::arrayValue;

                boost::optional<bool> anyAuthChanged(false);
                boost::optional<MAmount> totalSpend(0);

                for (auto const& tx : *txs)
                {
                    Json::Value jvTx = Json::objectValue;

                    jvTx[jss::seq] = tx.first;
                    jvTx[jss::fee_level] = to_string(tx.second.feeLevel);
                    if (tx.second.lastValid)
                        jvTx[jss::LastLedgerSequence] = *tx.second.lastValid;
                    if (tx.second.consequences)
                    {
                        jvTx[jss::fee] = to_string(
                            tx.second.consequences->fee);
                        auto spend = tx.second.consequences->potentialSpend +
                            tx.second.consequences->fee;
                        jvTx[jss::max_spend_drops] = to_string(spend);
                        if (totalSpend)
                            *totalSpend += spend;
                        auto authChanged = tx.second.consequences->category ==
                            TxConsequences::blocker;
                        if (authChanged)
                            anyAuthChanged.emplace(authChanged);
                        jvTx[jss::auth_change] = authChanged;
                    }
                    else
                    {
                        if (anyAuthChanged && !*anyAuthChanged)
                            anyAuthChanged.reset();
                        totalSpend.reset();
                    }

                    jvQueueTx.append(std::move(jvTx));
                }

                if (anyAuthChanged)
                    jvQueueData[jss::auth_change_queued] =
                        *anyAuthChanged;
                if (totalSpend)
                    jvQueueData[jss::max_spend_drops_total] =
                        to_string(*totalSpend);
            }
            else
                jvQueueData[jss::txn_count] = 0u;

            result[jss::queue_data] = std::move(jvQueueData);
        }
    }
    else
    {
        result[jss::account] = context.app.accountIDCache().toBase58 (accountID);
        RPC::inject_error (rpcACT_NOT_FOUND, result);
    }

    return result;
}

} //
