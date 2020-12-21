//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <mtchain/app/tx/impl/NFAsset.h>

#define MAX_ASSET_ID_LENGTH    15
#define MAX_ASSET_NAME_LENGTH  31

namespace mtchain {

TER CreateAsset::preflight (PreflightContext const& ctx)
{
    auto ret = preflight1 (ctx);

    if (!isTesSuccess (ret))
        return ret;

    auto const& tx = ctx.tx;
    auto const& id = tx.getFieldVL(sfIdent);
    if (id.size() > MAX_ASSET_ID_LENGTH)
    {
        return temBAD_ASSET;
    }

    if (tx.isFieldPresent(sfName))
    {
        auto const& name = tx.getFieldVL(sfName);
        if (name.size() > MAX_ASSET_NAME_LENGTH)
        {
            return temBAD_ASSET;
        }
    }

    if (tx.isFieldPresent(sfFlags) && (tx[sfFlags] & tfAssetCapMask))
    {
        return temBAD_ASSET;
    }

    return preflight2 (ctx);
}


TER CreateAsset::doApply ()
{
    auto const& tx = ctx_.tx;

    auto const& id = tx.getFieldVL(sfIdent);
    auto const& k = keylet::nfasset(account_, id);
    auto const& assetid = k.key;
    tx.addon[sfAssetID.getJsonName()] = to_string(assetid);

    if (view().exists (k))
    {
        return tefASSET_EXIST;
    }

    auto sleAsset = std::make_shared<SLE>(k);
    sleAsset->setAccountID(sfIssuer, account_);
    sleAsset->setFieldVL(sfIdent, id);
    sleAsset->setFieldH256(sfAssetID, assetid);
    sleAsset->setFieldU64(sfTokenNumber, 0);
    sleAsset->setFieldH256(sfTransactionHash, tx.getTransactionID());
    if (tx.isFieldPresent(sfName))
    {
        auto const& name = tx.getFieldVL(sfName);
        sleAsset->setFieldVL(sfName, name);
    }

    if (tx.isFieldPresent(sfMemos))
    {
        auto const& memos = tx.getFieldArray(sfMemos);
        sleAsset->setFieldArray(sfMemos, memos);
    }

    sleAsset->setFieldU32(sfFlags, tx.isFieldPresent(sfFlags) ? tx[sfFlags] : 0);
    view().insert (sleAsset);

    addAssetOwner(account_, assetid, view(), ctx_.journal);
    return tesSUCCESS;
}


TER ApproveAsset::preflight (PreflightContext const& ctx)
{
    auto ret = preflight1 (ctx);

    if (!isTesSuccess (ret))
        return ret;

    return preflight2 (ctx);
}


TER ApproveAsset::preclaim (PreclaimContext const& ctx)
{
    auto const& assetid = ctx.tx.getFieldH256(sfAssetID);
    if (!ctx.view.exists(keylet::nfasset(assetid)))
    {
        return tefNO_ASSET;
    }

    auto const& dest = ctx.tx.getAccountID(sfDestination);
    if (!ctx.view.exists(keylet::account(dest)))
    {
        return tecNO_DST;
    }

    if (dest == ctx.tx.getAccountID(sfAccount))
    {
        return temDST_IS_SRC;
    }
    
    return tesSUCCESS;

}


TER ApproveAsset::doApply ()
{
    auto const& k = *assetAuthKey_;
    auto sleAuth = view().peek(k);
    if (!sleAuth)
    {
        sleAuth = std::make_shared<SLE>(k);
        view().insert(sleAuth);
    }

    return tesSUCCESS;
}


TER CancelApproveAsset::doApply ()
{
    auto sleAuth = view().peek(*assetAuthKey_);
    if (sleAuth)
    {
        view().erase(sleAuth);
    }

    return tesSUCCESS;
}


TER DestroyAsset::preflight (PreflightContext const& ctx)
{
    auto ret = preflight1 (ctx);

    if (!isTesSuccess (ret))
        return ret;

    return preflight2 (ctx);
}


TER DestroyAsset::preclaim (PreclaimContext const& ctx)
{
    auto const& assetid = ctx.tx.getFieldH256(sfAssetID);
    auto sleAsset = ctx.view.read(keylet::nfasset(assetid));
    if (!sleAsset)
    {
        return tefNO_ASSET;
    }

    if (sleAsset->getAccountID(sfIssuer) != ctx.tx.getAccountID(sfAccount))
    {
        return tecNO_PERMISSION;
    }

    return tesSUCCESS;
}


TER DestroyAsset::doApply ()
{
    auto const& assetid = ctx_.tx.getFieldH256(sfAssetID);
    auto sleAsset = view().peek(keylet::nfasset(assetid));

    if (sleAsset->getFieldU64(sfTokenNumber) > 0)
    {
        return tefDESTROY_ASSET;
    }

    auto sleAssetOwner = view().peek(keylet::nfasset(assetid, account_));
    if (sleAssetOwner)
    {
        removeAssetOwner(account_, sleAssetOwner, view());
    }

    view().erase(sleAsset);

    return tesSUCCESS;
}


TER SetAsset::doApply ()
{
    auto const& tx = ctx_.tx;
    auto const& assetid = tx.getFieldH256(sfAssetID);
    auto sleAsset = view().peek(keylet::nfasset(assetid));
    auto flags = sleAsset->getFieldU32(sfFlags);

    if (tx.isFieldPresent(sfSetFlag))
    {
        if (tx.getFieldU32(sfSetFlag) == asfAllowTransferToken &&
            (flags & tfTransferToken) == 0)
        {
            if ((flags & tfToggleTransfer) == 0)
            {
                return tecNO_PERMISSION;
            }

            flags |= tfTransferToken;
        }
    } else if (tx.isFieldPresent(sfClearFlag))
    {
        if (tx.getFieldU32(sfClearFlag) == asfAllowTransferToken &&
            (flags & tfTransferToken) == 1)
        {
            if ((flags & tfToggleTransfer) == 0)
            {
                return tecNO_PERMISSION;
            }

            flags &= ~tfTransferToken;
        }
    }

    if (flags != sleAsset->getFieldU32(sfFlags))
    {
        sleAsset->setFieldU32(sfFlags, flags);
        view().update(sleAsset);
    }

    return tesSUCCESS;
}


std::shared_ptr<SLE> addAssetOwner(AccountID const& owner, uint256 const& assetid,
                                   ApplyView &view, beast::Journal const& j)
{
    auto sleOwner = view.peek(keylet::account(owner));
    auto assetNum = sleOwner->getFieldU64(sfAssetNumber);
    sleOwner->setFieldU64(sfAssetNumber, assetNum + 1);
    view.update(sleOwner);

    auto const& k = keylet::nfasset(owner, assetNum);
    auto sleAssetIndex = view.peek(k);
    if (!sleAssetIndex)
    {
        sleAssetIndex = std::make_shared<SLE>(k);
        view.insert(sleAssetIndex);
    }
    else
    {
        JLOG(j.warn()) << "AssetIndex has existed:"
                       << "AssetID = " << assetid << ", Account = " << owner
                       << ", AssetIndex = " << assetNum;
        view.update(sleAssetIndex);
    }
    sleAssetIndex->setFieldH256(sfAssetID, assetid);

    auto const& k1 = keylet::nfasset(assetid, owner);
    auto sleAssetOwner = std::make_shared<SLE>(k1);
    sleAssetOwner->setFieldU64(sfTokenNumber, 0);
    sleAssetOwner->setFieldU64(sfAssetIndex, assetNum);
    view.insert(sleAssetOwner);

    return sleAssetOwner;
}


void removeAssetOwner(AccountID const& owner, std::shared_ptr<SLE> sleAssetOwner, ApplyView &view)
{
    view.erase(sleAssetOwner);
    auto sleOwner = view.peek(keylet::account(owner));
    auto assetNum = sleOwner->getFieldU64(sfAssetNumber);
    if (assetNum > 0)
    {
        assetNum -= 1;
        sleOwner->setFieldU64(sfAssetNumber, assetNum);
        view.update(sleOwner);
    }

    if (!sleAssetOwner->isFieldPresent(sfAssetIndex))
        return;

    auto assetIndex = sleAssetOwner->getFieldU64(sfAssetIndex);
    auto sleAssetIndex = view.peek(keylet::nfasset(owner, assetIndex));
    //TODO: cann't find asset index entry
    if (!sleAssetIndex) return;

    if (assetNum != assetIndex)
    {
        auto sleAssetLastIndex = view.peek(keylet::nfasset(owner, assetNum));
        //TODO: cann't find last asset index entry
        if (!sleAssetLastIndex) return;

        auto const& lastAssetID = sleAssetLastIndex->getFieldH256(sfAssetID);
        sleAssetIndex->setFieldH256(sfAssetID, lastAssetID);
        view.erase(sleAssetLastIndex);
        view.update(sleAssetIndex);

        auto sleAssetOwner = view.peek(keylet::nfasset(lastAssetID, owner));
        if (sleAssetOwner)
        {
            sleAssetOwner->setFieldU64(sfAssetIndex, assetIndex);
            view.update(sleAssetOwner);
        }
    }
    else
    {
        view.erase(sleAssetIndex);
    }
}

} //
