//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/app/tx/impl/NFToken.h>
#include <mtchain/app/tx/impl/NFAsset.h>

#define MAX_TOKEN_ID_LENGTH   66
#define MAX_TOKEN_INFO_ENUM   16
#define MAX_TOKEN_INFO_SIZE   1024

namespace mtchain {

TER CreateToken::preflight (PreflightContext const& ctx)
{
    auto ret = preflight1 (ctx);

    if (!isTesSuccess (ret))
        return ret;

    auto const& id = ctx.tx.getFieldVL(sfIdent);
    if (id.size() > MAX_TOKEN_ID_LENGTH)
    {
        return temBAD_TOKEN;
    }

    return preflight2 (ctx);
}


TER CreateToken::preclaim (PreclaimContext const& ctx)
{
    auto const& owner = ctx.tx.getAccountID(sfOwner);
    if (!ctx.view.exists(keylet::account(owner)))
    {
        return tefNO_OWNER;
    }

    return tesSUCCESS;
}


TER CreateToken::doApply ()
{
    auto const& tx = ctx_.tx;

    auto const& assetid = tx.getFieldH256(sfAssetID);
    auto sleAsset = view().peek(keylet::nfasset(assetid));
    if (!sleAsset)
    {
        return tefNO_ASSET;
    }

    auto const& id = tx.getFieldVL(sfIdent);
    auto const& k = keylet::nftoken(assetid, id);
    tx.addon[sfTokenID.getJsonName()] = to_string(k.key);

    if (view().exists(k))
    {
        return tefTOKEN_EXIST;
    }

    auto const& issuer = sleAsset->getAccountID(sfIssuer);
    if (issuer != account_)
    {
        return tecNO_PERMISSION;
    }

    auto tokenNum = sleAsset->getFieldU64(sfTokenNumber);
    sleAsset->setFieldU64(sfTokenNumber, tokenNum + 1);
    view().update(sleAsset);

    auto const& owner = tx.getAccountID(sfOwner);
    std::uint64_t ownerTokenNum;
    {
        auto sleAssetOwner = view().peek(keylet::nfasset(assetid, owner));
        if (!sleAssetOwner)
        {
            ownerTokenNum = 0;
            sleAssetOwner = addAssetOwner(owner, assetid, view(), ctx_.journal);
        }
        else
        {
            ownerTokenNum = sleAssetOwner->getFieldU64(sfTokenNumber);
            view().update(sleAssetOwner);
        }

        sleAssetOwner->setFieldU64(sfTokenNumber, ownerTokenNum + 1);
    }

    {
        auto sleToken = std::make_shared<SLE>(k);
        sleToken->setFieldH256(sfAssetID, assetid);
        sleToken->setFieldVL(sfIdent, id);
        sleToken->setFieldH256(sfTokenID, k.key);
        sleToken->setAccountID(sfOwner, owner);
        sleToken->setFieldU64(sfTokenIndex, tokenNum);
        sleToken->setFieldU64(sfOwnerTokenIndex, ownerTokenNum);
        sleToken->setFieldH256(sfTransactionHash, tx.getTransactionID());
        if (tx.isFieldPresent(sfMemos))
        {
            auto const& memos = tx.getFieldArray(sfMemos);
            sleToken->setFieldArray(sfMemos, memos);
        }

        if (tx.isFieldPresent(sfImprint))
        {
            auto const& imprint = tx.getFieldVL(sfImprint);
            sleToken->setFieldVL(sfImprint, imprint);
        }

        view().insert(sleToken);
    }

    {
        auto const& k2 = keylet::nftoken(assetid, tokenNum);
        auto sleTokenIndex = view().peek(k2);
        if (!sleTokenIndex)
        {
            sleTokenIndex = std::make_shared<SLE>(k2);
            view().insert(sleTokenIndex);
        }
        else
        {
            JLOG(ctx_.journal.warn()) << "TokenIndex has existed: "
                                      << "AssetID = " << assetid << ", TokenIndex = " << tokenNum;
            view().update(sleTokenIndex);
        }
        sleTokenIndex->setFieldH256(sfTokenID, k.key);
    }

    {
        auto const& k3 = keylet::nftoken(assetid, owner, ownerTokenNum);
        auto sleOwnerTokenIndex = view().peek(k3);
        if (!sleOwnerTokenIndex)
        {
            sleOwnerTokenIndex = std::make_shared<SLE>(k3);
            view().insert(sleOwnerTokenIndex);
        }
        else
        {
            JLOG(ctx_.journal.warn()) << "OwnerTokenIndex has existed: "
                                      << "AssetID = " << assetid << ", Owner = " << owner
                                      << ", OwnerTokenIndex = " << ownerTokenNum;
            view().update(sleOwnerTokenIndex);
        }
        sleOwnerTokenIndex->setFieldH256(sfTokenID, k.key);
    }

    return tesSUCCESS;
}


TER DestroyToken::preflight (PreflightContext const& ctx)
{
    auto ret = preflight1 (ctx);

    if (!isTesSuccess (ret))
        return ret;

    if (ctx.tx.isFieldPresent(sfTokenID) ||
        (ctx.tx.isFieldPresent(sfAssetID) && ctx.tx.isFieldPresent(sfIdent)))
    {
        return preflight2 (ctx);        
    }

    return temMALFORMED;
}


TER DestroyToken::doApply ()
{
    auto const& k = *tokenKey_;
    auto sleToken = view().peek(k);
    if (!sleToken)
    {
        return tefNO_TOKEN;
    }

    auto const& assetid = sleToken->getFieldH256(sfAssetID);
    auto sleAsset = view().peek(keylet::nfasset(assetid));
    if (!sleAsset)
    {
        JLOG(ctx_.journal.warn()) << "Token's asset doesn't exist: "
                                  << "TokenID = " << k.key << ", AssetID = " << assetid;
        return tefINTERNAL;
    }

    if (!checkPermission(sleToken, sleAsset))
    {
        return tecNO_PERMISSION;
    }

    auto const& owner = sleToken->getAccountID(sfOwner);
    auto sleAssetOwner = view().peek(keylet::nfasset(assetid, owner));
    if (!sleAssetOwner)
    {
        JLOG(ctx_.journal.warn()) << "Asset's owner info doesn't exist: "
                                  << "AssetID = " << assetid << ", Owner = " << owner;
        return tefINTERNAL;
    }

    auto tokenIndex = sleToken->getFieldU64(sfTokenIndex);
    auto sleTokenIndex = view().peek(keylet::nftoken(assetid, tokenIndex));
    if (!sleTokenIndex)
    {
        JLOG(ctx_.journal.warn()) << "TokenIndex doesn't exist: "
                                  << "AssetID = " << assetid << ", TokenIndex = " << tokenIndex
                                  << ", TokenID = " << k.key;
        return tefINTERNAL;
    }

    auto ownerTokenIndex = sleToken->getFieldU64(sfOwnerTokenIndex);
    auto sleOwnerTokenIndex = view().peek(keylet::nftoken(assetid, owner, ownerTokenIndex));
    if (!sleOwnerTokenIndex)
    {
        JLOG(ctx_.journal.warn()) << "OwnerTokenIndex doesn't exist: "
                                  << "AssetID = " << assetid << ", Owner = " << owner
                                  << ", OwnerTokenIndex = " << ownerTokenIndex
                                  << ", TokenID = " << k.key;
        return tefINTERNAL;
    }

    auto tokenNum = sleAsset->getFieldU64(sfTokenNumber) - 1;
    auto sleLastTokenIndex = view().peek(keylet::nftoken(assetid, tokenNum));
    if (!sleLastTokenIndex)
    {
        JLOG(ctx_.journal.warn()) << "LastTokenIndex doesn't exist: "
                                  << "AssetID = " << assetid << ", LastTokenIndex = " << tokenNum;
        return tefINTERNAL;
    }

    auto const& lastTokenID = sleLastTokenIndex->getFieldH256(sfTokenID);
    auto sleLastToken = view().peek(keylet::nftoken(lastTokenID));
    if (!sleLastToken)
    {
        JLOG(ctx_.journal.warn()) << "LastToken doesn't exist: "
                                  << "AssetID = " << assetid << ", LastTokenIndex = " << tokenNum
                                  << ", LastTokenID = " << lastTokenID;
        return tefINTERNAL;
    }

    auto ownerTokenNum = sleAssetOwner->getFieldU64(sfTokenNumber) - 1;
    auto sleLastOwnerTokenIndex = view().peek(keylet::nftoken(assetid, owner, ownerTokenNum));
    if (!sleLastOwnerTokenIndex)
    {
        JLOG(ctx_.journal.warn()) << "LastOwnerTokenIndex doesn't exist: "
                                  << "AssetID = " << assetid << ", Owner = " << owner
                                  << ", LastOwnerTokenIndex = " << ownerTokenNum;
        return tefINTERNAL;
    }

    auto const& lastOwnerTokenID = sleLastOwnerTokenIndex->getFieldH256(sfTokenID);
    auto sleLastOwnerToken = view().peek(keylet::nftoken(lastOwnerTokenID));
    if (!sleLastOwnerToken)
    {
        JLOG(ctx_.journal.warn()) << "LastOwnerToken doesn't exist: "
                                  << "AssetID = " << assetid << ", Owner = " << owner
                                  << ", LastOwnerTokenIndex = " << ownerTokenNum
                                  << ", LastOwnerTokenID = " << lastOwnerTokenID;
        return tefINTERNAL;
    }

    sleAsset->setFieldU64(sfTokenNumber, tokenNum);
    view().update(sleAsset);

    view().erase(sleToken);

    if (ownerTokenNum != 0 || owner == sleAsset->getAccountID(sfIssuer))
    {
        sleAssetOwner->setFieldU64(sfTokenNumber, ownerTokenNum);
        view().update(sleAssetOwner);
    }
    else
    {
        removeAssetOwner(owner, sleAssetOwner, view());
    }

    view().erase(sleLastTokenIndex);
    view().erase(sleLastOwnerTokenIndex);

    if (tokenIndex != tokenNum)
    {
        sleTokenIndex->setFieldH256(sfTokenID, lastTokenID);
        view().update(sleTokenIndex);

        sleLastToken->setFieldU64(sfTokenIndex, tokenIndex);
        view().update(sleLastToken);
    }

    if (ownerTokenIndex != ownerTokenNum)
    {
        sleOwnerTokenIndex->setFieldH256(sfTokenID, lastOwnerTokenID);
        view().update(sleOwnerTokenIndex);

        sleLastOwnerToken->setFieldU64(sfOwnerTokenIndex, ownerTokenIndex);
        view().update(sleLastOwnerToken);
    }

    return tesSUCCESS;
}


TER TransferToken::preclaim (PreclaimContext const& ctx)
{
    auto const& dest = ctx.tx.getAccountID(sfDestination);
    if (!ctx.view.exists(keylet::account(dest)))
    {
        return tecNO_DST;
    }

    return tesSUCCESS;
}


TER TransferToken::doApply ()
{
    auto const& k = *tokenKey_;
    auto sleToken = view().peek(k);
    if (!sleToken)
    {
        return tefNO_TOKEN;
    }

    auto const& assetid = sleToken->getFieldH256(sfAssetID);
    auto sleAsset = view().peek(keylet::nfasset(assetid));
    if (!sleAsset)
    {
        JLOG(ctx_.journal.warn()) << "Token's asset doesn't exist: "
                                  << "TokenID = " << k.key << ", AssetID = " << assetid;
        return tefINTERNAL;
    }

    if (!checkPermission(sleToken, sleAsset))
    {
        return tecNO_PERMISSION;
    }

    auto const& owner = sleToken->getAccountID(sfOwner);
    auto const& dest = ctx_.tx.getAccountID(sfDestination);
    if (dest == owner)
    {
        return tefDST_IS_OWNER;
    }

    auto sleAssetOwner = view().peek(keylet::nfasset(assetid, owner));
    if (!sleAssetOwner)
    {
        JLOG(ctx_.journal.warn()) << "Asset's owner info doesn't exist: "
                                  << "AssetID = " << assetid << ", Owner = " << owner;
        return tefINTERNAL;
    }

    auto ownerTokenIndex = sleToken->getFieldU64(sfOwnerTokenIndex);
    auto sleOwnerTokenIndex = view().peek(keylet::nftoken(assetid, owner, ownerTokenIndex));
    if (!sleOwnerTokenIndex)
    {
        JLOG(ctx_.journal.warn()) << "OwnerTokenIndex doesn't exist: "
                                  << "AssetID = " << assetid << ", OwnerTokenIndex = "
                                  << ownerTokenIndex << ", TokenID = " << k.key;
        return tefINTERNAL;
    }

    auto ownerTokenNum = sleAssetOwner->getFieldU64(sfTokenNumber) - 1;
    auto sleLastOwnerTokenIndex = view().peek(keylet::nftoken(assetid, owner, ownerTokenNum));
    if (!sleLastOwnerTokenIndex)
    {
        JLOG(ctx_.journal.warn()) << "LastOwnerTokenIndex doesn't exist: "
                                  << "AssetID = " << assetid << ", Owner = " << owner
                                  << ", LastOwnerTokenIndex = " << ownerTokenNum;
        return tefINTERNAL;
    }

    auto const& lastOwnerTokenID = sleLastOwnerTokenIndex->getFieldH256(sfTokenID);
    auto sleLastOwnerToken = view().peek(keylet::nftoken(lastOwnerTokenID));
    if (!sleLastOwnerToken)
    {
        JLOG(ctx_.journal.warn()) << "LastOwnerToken doesn't exist: "
                                  << "AssetID = " << assetid << ", Owner = " << owner
                                  << ", LastOwnerTokenIndex = " << ownerTokenNum
                                  << ", LastOwnerTokenID = " << lastOwnerTokenID;
        return tefINTERNAL;
    }

    // remove the token from the current owner
    if (ownerTokenNum != 0 || owner == sleAsset->getAccountID(sfIssuer))
    {
        sleAssetOwner->setFieldU64(sfTokenNumber, ownerTokenNum);
        view().update(sleAssetOwner);
    }
    else
    {
        removeAssetOwner(owner, sleAssetOwner, view());
    }

    view().erase(sleLastOwnerTokenIndex);

    if (ownerTokenIndex != ownerTokenNum)
    {
        sleOwnerTokenIndex->setFieldH256(sfTokenID, lastOwnerTokenID);
        view().update(sleOwnerTokenIndex);

        sleLastOwnerToken->setFieldU64(sfOwnerTokenIndex, ownerTokenIndex);
        view().update(sleLastOwnerToken);
    }

    // add the token to the new owner
    {
        sleAssetOwner = view().peek(keylet::nfasset(assetid, dest));
        if (!sleAssetOwner)
        {
            ownerTokenNum = 0;
            sleAssetOwner = addAssetOwner(dest, assetid, view(), ctx_.journal);
        }
        else
        {
            ownerTokenNum = sleAssetOwner->getFieldU64(sfTokenNumber);
            view().update(sleAssetOwner);
        }
        sleAssetOwner->setFieldU64(sfTokenNumber, ownerTokenNum + 1);
    }

    {
        auto const& k2 = keylet::nftoken(assetid, dest, ownerTokenNum);
        sleOwnerTokenIndex = view().peek(k2);
        if (!sleOwnerTokenIndex)
        {
            sleOwnerTokenIndex = std::make_shared<SLE>(k2);
            view().insert(sleOwnerTokenIndex);
        }
        else
        {
            JLOG(ctx_.journal.warn()) << "OwnerTokenIndex has existed: "
                                      << "AssetID = " << assetid << ", Owner =  " << owner
                                      << ", OwnerTokenIndex = " << ownerTokenNum;
            view().update(sleOwnerTokenIndex);
        }
        sleOwnerTokenIndex->setFieldH256(sfTokenID, k.key);
    }

    sleToken->setAccountID(sfOwner, dest);
    sleToken->setFieldU64(sfOwnerTokenIndex, ownerTokenNum);
    if (sleToken->isFieldPresent(sfApproved))
    {
        sleToken->makeFieldAbsent(sfApproved);
    }
    view().update(sleToken);

    return tesSUCCESS;
}


TER ApproveToken::doApply ()
{
    auto sleToken = view().peek(*tokenKey_);
    if (!sleToken)
    {
        return tefNO_TOKEN;
    }

    if (!checkPermission(sleToken))
    {
        return tecNO_PERMISSION;
    }

    auto const& dest = ctx_.tx.getAccountID(sfDestination);
    if (dest == sleToken->getAccountID(sfOwner))
    {
        return temDST_IS_SRC;
    }

    if (!sleToken->isFieldPresent(sfApproved) || dest != sleToken->getAccountID(sfApproved))
    {
        sleToken->setAccountID(sfApproved, dest);
        view().update(sleToken);
    }

    return tesSUCCESS;
}


TER CancelApproveToken::doApply ()
{
    auto sleToken = view().peek(*tokenKey_);
    if (!sleToken)
    {
        return tefNO_TOKEN;
    }

    if (!checkPermission(sleToken))
    {
        return tecNO_PERMISSION;
    }

    if (sleToken->isFieldPresent(sfApproved))
    {
        sleToken->makeFieldAbsent(sfApproved);
        view().update(sleToken);
    }

    return tesSUCCESS;
}


static bool updateMemos(STArray &tokenInfos, STObject const& memo)
{
    auto const& type = memo.getFieldVL(sfMemoType);
    for (auto itr = tokenInfos.begin(); itr != tokenInfos.end(); ++itr)
    {
        auto &info = *itr;
        if (info.getFieldVL(sfMemoType) != type)
            continue;

        auto const& data = memo.getFieldVL(sfMemoData);
        if (data.size() > 0)
        {
            if (data == info.getFieldVL(sfMemoData))
                return false;

            info = memo;
        }
        else
        {
            tokenInfos.erase(itr);
        }

        return true;
    }

    tokenInfos.push_back(memo);
    return true;
}


TER SetToken::doApply ()
{
    auto const& k = *tokenKey_;
    auto sleToken = view().peek(k);
    if (!sleToken)
    {
        return tefNO_TOKEN;
    }

    if (!checkPermission(sleToken))
    {
        return tecNO_PERMISSION;
    }

    bool changed = false;
    auto const& tx = ctx_.tx;
    if (tx.isFieldPresent(sfMemos))
    {
        auto &tokenInfos = sleToken->peekFieldArray(sfMemos);
        auto const& memos = tx.getFieldArray(sfMemos);
        for (auto const& memo : memos)
        {
            changed |= updateMemos(tokenInfos, memo);
        }

        if (changed)
        {
            if (tokenInfos.size() > MAX_TOKEN_INFO_ENUM)
            {
                return tecINTERNAL;
            }

            if (tokenInfos.size() == 0)
            {
                sleToken->makeFieldAbsent(sfMemos);
            }
            else
            {
                Serializer s(MAX_TOKEN_INFO_SIZE);
                tokenInfos.add(s);
                if (s.getDataLength() > MAX_TOKEN_INFO_SIZE)
                {
                    return tecINTERNAL;
                }
            }
        }
    }

    if (tx.isFieldPresent(sfImprint))
    {
        if (sleToken->isFieldPresent(sfImprint))
        {
            return tecNO_PERMISSION;
        }

        changed = true;
        auto const& imprint = tx.getFieldVL(sfImprint);
        sleToken->setFieldVL(sfImprint, imprint);
    }

    if (changed) view().update(sleToken);
    return tesSUCCESS;
}

} //
