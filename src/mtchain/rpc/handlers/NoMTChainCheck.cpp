//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/LoadFeeTrack.h>
#include <mtchain/app/paths/MTChainState.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/TxFlags.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>
#include <mtchain/rpc/impl/Tuning.h>

namespace mtchain {

static void fillTransaction (
    RPC::Context& context,
    Json::Value& txArray,
    AccountID const& accountID,
    std::uint32_t& sequence,
    ReadView const& ledger)
{
    txArray["Sequence"] = Json::UInt (sequence++);
    txArray["Account"] = context.app.accountIDCache().toBase58 (accountID);
    auto& fees = ledger.fees();
    // Convert the reference transaction cost in fee units to drops
    // scaled to represent the current fee load.
    txArray["Fee"] = Json::UInt (scaleFeeLoad(fees.units,
        context.app.getFeeTrack(), fees, false));
}

// {
//   account: <account>|<account_public_key>
//   ledger_hash : <ledger>
//   ledger_index : <ledger_index>
//   limit: integer                 // optional, number of problems
//   role: gateway|user             // account role to assume
//   transactions: true             // optional, reccommend transactions
// }
Json::Value doNoMTChainCheck (RPC::Context& context)
{
    auto const& params (context.params);
    if (! params.isMember (jss::account))
        return RPC::missing_field_error ("account");

    if (! params.isMember ("role"))
        return RPC::missing_field_error ("role");
    bool roleGateway = false;
    {
        std::string const role = params["role"].asString();
        if (role == "gateway")
            roleGateway = true;
        else if (role != "user")
        return RPC::invalid_field_message ("role");
    }

    unsigned int limit;
    if (auto err = readLimitField(limit, RPC::Tuning::noMtchainCheck, context))
        return *err;

    bool transactions = false;
    if (params.isMember (jss::transactions))
        transactions = params["transactions"].asBool();

    std::shared_ptr<ReadView const> ledger;
    auto result = RPC::lookupLedger (ledger, context);
    if (! ledger)
        return result;

    Json::Value dummy;
    Json::Value& jvTransactions =
        transactions ? (result[jss::transactions] = Json::arrayValue) : dummy;

    std::string strIdent (params[jss::account].asString ());
    AccountID accountID;

    if (auto jv = RPC::accountFromString (accountID, strIdent))
    {
        for (auto it (jv.begin ()); it != jv.end (); ++it)
            result[it.memberName ()] = it.key ();

        return result;
    }

    auto const sle = ledger->read(keylet::account(accountID));
    if (! sle)
        return rpcError (rpcACT_NOT_FOUND);

    std::uint32_t seq = sle->getFieldU32 (sfSequence);

    Json::Value& problems = (result["problems"] = Json::arrayValue);

    bool bDefaultMtchain = sle->getFieldU32 (sfFlags) & lsfDefaultMtchain;

    if (bDefaultMtchain & ! roleGateway)
    {
        problems.append ("You appear to have set your default mtchain flag even though you "
            "are not a gateway. This is not recommended unless you are experimenting");
    }
    else if (roleGateway & ! bDefaultMtchain)
    {
        problems.append ("You should immediately set your default mtchain flag");
        if (transactions)
        {
            Json::Value& tx = jvTransactions.append (Json::objectValue);
            tx["TransactionType"] = "AccountSet";
            tx["SetFlag"] = 8;
            fillTransaction (context, tx, accountID, seq, *ledger);
        }
    }

    forEachItemAfter (*ledger, accountID,
            uint256(), 0, limit,
        [&](std::shared_ptr<SLE const> const& ownedItem)
        {
            if (ownedItem->getType() == ltMTCHAIN_STATE)
            {
                bool const bLow = accountID == ownedItem->getFieldAmount(sfLowLimit).getIssuer();

                bool const bNoMTChain = ownedItem->getFieldU32(sfFlags) &
                    (bLow ? lsfLowNoMtchain : lsfHighNoMtchain);

                std::string problem;
                bool needFix = false;
                if (bNoMTChain & roleGateway)
                {
                    problem = "You should clear the no mtchain flag on your ";
                    needFix = true;
                }
                else if (! roleGateway & ! bNoMTChain)
                {
                    problem = "You should probably set the no mtchain flag on your ";
                    needFix = true;
                }
                if (needFix)
                {
                    AccountID peer =
                        ownedItem->getFieldAmount (bLow ? sfHighLimit : sfLowLimit).getIssuer();
                    STAmount peerLimit = ownedItem->getFieldAmount (bLow ? sfHighLimit : sfLowLimit);
                    problem += to_string (peerLimit.getCurrency());
                    problem += " line to ";
                    problem += to_string (peerLimit.getIssuer());
                    problems.append (problem);

                    STAmount limitAmount (ownedItem->getFieldAmount (bLow ? sfLowLimit : sfHighLimit));
                    limitAmount.setIssuer (peer);

                    Json::Value& tx = jvTransactions.append (Json::objectValue);
                    tx["TransactionType"] = "TrustSet";
                    tx["LimitAmount"] = limitAmount.getJson (0);
                    tx["Flags"] = bNoMTChain ? tfClearNoMtchain : tfSetNoMtchain;
                    fillTransaction(context, tx, accountID, seq, *ledger);

                    return true;
                }
            }
            return false;
        });

    return result;
}

} //
