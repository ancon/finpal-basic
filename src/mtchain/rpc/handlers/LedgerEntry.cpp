//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/basics/strHex.h>
#include <mtchain/basics/StringUtilities.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/net/RPCErr.h>
#include <mtchain/protocol/ErrorCodes.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/rpc/Context.h>
#include <mtchain/rpc/impl/RPCHelpers.h>

namespace mtchain {

// {
//   ledger_hash : <ledger>
//   ledger_index : <ledger_index>
//   ...
// }
Json::Value doLedgerEntry (RPC::Context& context)
{
    std::shared_ptr<ReadView const> lpLedger;
    auto jvResult = RPC::lookupLedger (lpLedger, context);

    if (!lpLedger)
        return jvResult;

    uint256     uNodeIndex;
    bool        bNodeBinary = false;

    if (context.params.isMember (jss::index))
    {
        // XXX Needs to provide proof.
        uNodeIndex.SetHex (context.params[jss::index].asString ());
        bNodeBinary = true;
    }
    else if (context.params.isMember (jss::account_root))
    {
        auto const account = parseBase58<AccountID>(
            context.params[jss::account_root].asString());
        if (! account || account->isZero())
            jvResult[jss::error]   = "malformedAddress";
        else
            uNodeIndex = keylet::account(*account).key;
    }
    else if (context.params.isMember (jss::directory))
    {
        if (!context.params[jss::directory].isObject ())
        {
            uNodeIndex.SetHex (context.params[jss::directory].asString ());
        }
        else if (context.params[jss::directory].isMember (jss::sub_index)
                 && !context.params[jss::directory][jss::sub_index].isIntegral ())
        {
            jvResult[jss::error]   = "malformedRequest";
        }
        else
        {
            std::uint64_t  uSubIndex
                    = context.params[jss::directory].isMember (jss::sub_index)
                    ? context.params[jss::directory][jss::sub_index].asUInt () : 0;

            if (context.params[jss::directory].isMember (jss::dir_root))
            {
                uint256 uDirRoot;

                uDirRoot.SetHex (context.params[jss::dir_root].asString ());

                uNodeIndex  = getDirNodeIndex (uDirRoot, uSubIndex);
            }
            else if (context.params[jss::directory].isMember (jss::owner))
            {
                auto const ownerID = parseBase58<AccountID>(
                    context.params[jss::directory][jss::owner].asString());

                if (! ownerID)
                {
                    jvResult[jss::error]   = "malformedAddress";
                }
                else
                {
                    uint256 uDirRoot = getOwnerDirIndex (*ownerID);
                    uNodeIndex  = getDirNodeIndex (uDirRoot, uSubIndex);
                }
            }
            else
            {
                jvResult[jss::error]   = "malformedRequest";
            }
        }
    }
    else if (context.params.isMember (jss::generator))
    {
        jvResult[jss::error]   = "deprecatedFeature";
    }
    else if (context.params.isMember (jss::offer))
    {
        if (!context.params[jss::offer].isObject ())
        {
            uNodeIndex.SetHex (context.params[jss::offer].asString ());
        }
        else if (!context.params[jss::offer].isMember (jss::account)
                 || !context.params[jss::offer].isMember (jss::seq)
                 || !context.params[jss::offer][jss::seq].isIntegral ())
        {
            jvResult[jss::error]   = "malformedRequest";
        }
        else
        {
            auto const id = parseBase58<AccountID>(
                context.params[jss::offer][jss::account].asString());
            if (! id)
                jvResult[jss::error]   = "malformedAddress";
            else
                uNodeIndex  = getOfferIndex (*id,
                    context.params[jss::offer][jss::seq].asUInt ());
        }
    }
    else if (context.params.isMember (jss::FinPal_state))
    {
        Currency        uCurrency;
        Json::Value     jvMTChainState   = context.params[jss::FinPal_state];

        if (!jvMTChainState.isObject ()
            || !jvMTChainState.isMember (jss::currency)
            || !jvMTChainState.isMember (jss::accounts)
            || !jvMTChainState[jss::accounts].isArray ()
            || 2 != jvMTChainState[jss::accounts].size ()
            || !jvMTChainState[jss::accounts][0u].isString ()
            || !jvMTChainState[jss::accounts][1u].isString ()
            || (jvMTChainState[jss::accounts][0u].asString ()
                == jvMTChainState[jss::accounts][1u].asString ())
           )
        {
            jvResult[jss::error]   = "malformedRequest";
        }
        else
        {
            auto const id1 = parseBase58<AccountID>(
                jvMTChainState[jss::accounts][0u].asString());
            auto const id2 = parseBase58<AccountID>(
                jvMTChainState[jss::accounts][1u].asString());
            if (! id1 || ! id2)
            {
                jvResult[jss::error]   = "malformedAddress";
            }
            else if (!to_currency (uCurrency,
                jvMTChainState[jss::currency].asString()))
            {
                jvResult[jss::error]   = "malformedCurrency";
            }
            else
            {
                uNodeIndex  = getMtchainStateIndex(
                    *id1, *id2, uCurrency);
            }
        }
    }
    else
    {
        jvResult[jss::error]   = "unknownOption";
    }

    if (uNodeIndex.isNonZero ())
    {
        auto const sleNode = lpLedger->read(keylet::unchecked(uNodeIndex));
        if (context.params.isMember(jss::binary))
            bNodeBinary = context.params[jss::binary].asBool();

        if (!sleNode)
        {
            // Not found.
            // XXX Should also provide proof.
            jvResult[jss::error]       = "entryNotFound";
        }
        else if (bNodeBinary)
        {
            // XXX Should also provide proof.
            Serializer s;

            sleNode->add (s);

            jvResult[jss::node_binary] = strHex (s.peekData ());
            jvResult[jss::index]       = to_string (uNodeIndex);
        }
        else
        {
            jvResult[jss::node]        = sleNode->getJson (0);
            jvResult[jss::index]       = to_string (uNodeIndex);
        }
    }

    return jvResult;
}


// ledger_entry_index <LedgerEntryType> <Parameters>
Json::Value doLedgerEntryIndex (RPC::Context& context)
{
    LedgerEntryType const type = LedgerFormats::getInstance().findTypeByName(
        context.params[sfLedgerEntryType.getJsonName()].asString());

    Json::Value jvResult(Json::objectValue);
    Json::Value const& args = context.params[jss::args];

    jvResult[sfLedgerEntryType.getJsonName()] = context.params[sfLedgerEntryType.getJsonName()];
    switch (type)
    {
    case ltACCOUNT_ROOT: {
        if (args.size() != 1)
            return rpcError(rpcINVALID_PARAMS);

        auto account = parseBase58<AccountID>(args[0u].asString());
        if (!account) return rpcError(rpcINVALID_PARAMS);

        jvResult[sfAccount.getJsonName()] = args[0u];
        jvResult[jss::index] = to_string(keylet::account(*account).key);
        break;
    }
    case ltNFASSET: {
        if (args.size() != 2)
            return rpcError(rpcINVALID_PARAMS);

        auto issuer = parseBase58<AccountID>(args[0u].asString());
        if (!issuer) return rpcError(rpcINVALID_PARAMS);

        auto id = to_blob(args[1u].asString());

        jvResult[sfIssuer.getJsonName()] = args[0u];
        jvResult[jss::id] = args[1u];
        jvResult[jss::index] = to_string(keylet::nfasset(*issuer, id).key);
        break;
    }
    case ltNFTOKEN: {
        if (args.size() != 2)
            return rpcError(rpcINVALID_PARAMS);

        uint256 assetid;
        if (!assetid.SetHex(args[0u].asString()))
            return rpcError(rpcINVALID_PARAMS);

        auto id = to_blob(args[1u].asString());

        jvResult[sfAssetID.getJsonName()] = args[0u];
        jvResult[jss::id] = args[1u];
        jvResult[jss::index] = to_string(keylet::nftoken(assetid, id).key);
        break;
    }
    case ltNFT_ACCOUNT: {
        if (args.size() != 2)
            return rpcError(rpcINVALID_PARAMS);

        uint256 assetid;
        if (!assetid.SetHex(args[0u].asString()))
            return rpcError(rpcINVALID_PARAMS);

        auto owner = parseBase58<AccountID>(args[1u].asString());
        if (!owner) return rpcError(rpcINVALID_PARAMS);

        jvResult[sfAssetID.getJsonName()] = args[0u];
        jvResult[sfOwner.getJsonName()] = args[1u];
        jvResult[jss::index] = to_string(keylet::nfasset(assetid, *owner).key);
        break;
    }
    case ltNFT_INDEX: {
        if (args.size() != 2)
            return rpcError(rpcINVALID_PARAMS);

        uint256 assetid;
        if (!assetid.SetHex(args[0u].asString()))
            return rpcError(rpcINVALID_PARAMS);

        std::uint64_t tokenIndex = beast::lexicalCastThrow<std::uint64_t>(args[1u].asString());

        jvResult[sfAssetID.getJsonName()] = args[0u];
        jvResult[sfTokenIndex.getJsonName()] = args[1u];
        jvResult[jss::index] = to_string(keylet::nftoken(assetid, tokenIndex).key);
        break;
    }
    case ltNFT_OWNER_INDEX: {
        if (args.size() != 3)
            return rpcError(rpcINVALID_PARAMS);

        uint256 assetid;
        if (!assetid.SetHex(args[0u].asString()))
            return rpcError(rpcINVALID_PARAMS);

        auto owner = parseBase58<AccountID>(args[1u].asString());
        if (!owner) return rpcError(rpcINVALID_PARAMS);

        std::uint64_t ownerTokenIndex = beast::lexicalCastThrow<std::uint64_t>(args[2u].asString());

        jvResult[sfAssetID.getJsonName()] = args[0u];
        jvResult[sfOwner.getJsonName()] = args[1u];
        jvResult[sfOwnerTokenIndex.getJsonName()] = args[2u];
        jvResult[jss::index] = to_string(keylet::nftoken(assetid, *owner, ownerTokenIndex).key);
        break;
    }
    case ltNFTAUTH: {
        if (args.size() != 3)
            return rpcError(rpcINVALID_PARAMS);

        uint256 assetid;
        if (!assetid.SetHex(args[0u].asString()))
            return rpcError(rpcINVALID_PARAMS);

        auto owner = parseBase58<AccountID>(args[1u].asString());
        if (!owner) return rpcError(rpcINVALID_PARAMS);

        auto oper = parseBase58<AccountID>(args[2u].asString());
        if (!oper) return rpcError(rpcINVALID_PARAMS);

        jvResult[sfAssetID.getJsonName()] = args[0u];
        jvResult[sfOwner.getJsonName()] = args[1u];
        jvResult[sfOperator.getJsonName()] = args[2u];
        jvResult[jss::index] = to_string(keylet::nfasset(assetid, *owner, *oper).key);
        break;
    }
    case ltNFASSET_INDEX: {
        if (args.size() != 2)
            return rpcError(rpcINVALID_PARAMS);

        auto account = parseBase58<AccountID>(args[0u].asString());
        if (!account) return rpcError(rpcINVALID_PARAMS);

        std::uint64_t assetIndex = beast::lexicalCastThrow<std::uint64_t>(args[1u].asString());

        jvResult[jss::Account] = args[0u];
        jvResult[sfAssetIndex.getJsonName()] = args[1u];
        jvResult[jss::index] = to_string(keylet::nfasset(*account, assetIndex).key);
        break;
    }
    default:
        return rpcError(rpcINVALID_PARAMS);
    }

    return jvResult;
}

} //
