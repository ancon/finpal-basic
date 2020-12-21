//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/paths/MTChainState.h>
#include <mtchain/protocol/STAmount.h>
#include <cstdint>
#include <memory>

namespace mtchain {

MTChainState::pointer
MTChainState::makeItem (
    AccountID const& accountID,
        std::shared_ptr<SLE const> sle)
{
    // VFALCO Does this ever happen in practice?
    if (! sle || sle->getType () != ltMTCHAIN_STATE)
        return {};
    return std::make_shared<MTChainState>(
        std::move(sle), accountID);
}

MTChainState::MTChainState (
    std::shared_ptr<SLE const>&& sle,
        AccountID const& viewAccount)
    : sle_ (std::move(sle))
    , mFlags (sle_->getFieldU32 (sfFlags))
    , mLowLimit (sle_->getFieldAmount (sfLowLimit))
    , mHighLimit (sle_->getFieldAmount (sfHighLimit))
    , mLowID (mLowLimit.getIssuer ())
    , mHighID (mHighLimit.getIssuer ())
    , lowQualityIn_ (sle_->getFieldU32 (sfLowQualityIn))
    , lowQualityOut_ (sle_->getFieldU32 (sfLowQualityOut))
    , highQualityIn_ (sle_->getFieldU32 (sfHighQualityIn))
    , highQualityOut_ (sle_->getFieldU32 (sfHighQualityOut))
    , mBalance (sle_->getFieldAmount (sfBalance))
{
    mViewLowest = (mLowID == viewAccount);

    if (!mViewLowest)
        mBalance.negate ();
}

Json::Value MTChainState::getJson (int)
{
    Json::Value ret (Json::objectValue);
    ret["low_id"] = to_string (mLowID);
    ret["high_id"] = to_string (mHighID);
    return ret;
}

std::vector <MTChainState::pointer>
getMTChainStateItems (AccountID const& accountID,
    ReadView const& view)
{
    std::vector <MTChainState::pointer> items;
    forEachItem(view, accountID,
        [&items,&accountID](
        std::shared_ptr<SLE const> const&sleCur)
        {
             auto ret = MTChainState::makeItem (accountID, sleCur);
             if (ret)
                items.push_back (ret);
        });

    return items;
}

} //
