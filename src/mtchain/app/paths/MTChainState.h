//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_MTCHAINSTATE_H_INCLUDED
#define MTCHAIN_APP_PATHS_MTCHAINSTATE_H_INCLUDED

#include <mtchain/ledger/View.h>
#include <mtchain/protocol/Rate.h>
#include <mtchain/protocol/STAmount.h>
#include <mtchain/protocol/STLedgerEntry.h>
#include <cstdint>
#include <memory> // <memory>

namespace mtchain {

/** Wraps a trust line SLE for convenience.
    The complication of trust lines is that there is a
    "low" account and a "high" account. This wraps the
    SLE and expresses its data from the perspective of
    a chosen account on the line.
*/
// VFALCO TODO Rename to TrustLine
class MTChainState
{
public:
    // VFALCO Why is this shared_ptr?
    using pointer = std::shared_ptr <MTChainState>;

public:
    MTChainState () = delete;

    virtual ~MTChainState() = default;

    static MTChainState::pointer makeItem(
        AccountID const& accountID,
        std::shared_ptr<SLE const> sle);

    // Must be public, for make_shared
    MTChainState (std::shared_ptr<SLE const>&& sle,
        AccountID const& viewAccount);

    /** Returns the state map key for the ledger entry. */
    uint256
    key() const
    {
        return sle_->key();
    }

    // VFALCO Take off the "get" from each function name

    AccountID const& getAccountID () const
    {
        return  mViewLowest ? mLowID : mHighID;
    }

    AccountID const& getAccountIDPeer () const
    {
        return !mViewLowest ? mLowID : mHighID;
    }

    // True, Provided auth to peer.
    bool getAuth () const
    {
        return mFlags & (mViewLowest ? lsfLowAuth : lsfHighAuth);
    }

    bool getAuthPeer () const
    {
        return mFlags & (!mViewLowest ? lsfLowAuth : lsfHighAuth);
    }

    bool getNoMtchain () const
    {
        return mFlags & (mViewLowest ? lsfLowNoMtchain : lsfHighNoMtchain);
    }

    bool getNoMtchainPeer () const
    {
        return mFlags & (!mViewLowest ? lsfLowNoMtchain : lsfHighNoMtchain);
    }

    /** Have we set the freeze flag on our peer */
    bool getFreeze () const
    {
        return mFlags & (mViewLowest ? lsfLowFreeze : lsfHighFreeze);
    }

    /** Has the peer set the freeze flag on us */
    bool getFreezePeer () const
    {
        return mFlags & (!mViewLowest ? lsfLowFreeze : lsfHighFreeze);
    }

    STAmount const& getBalance () const
    {
        return mBalance;
    }

    STAmount const& getLimit () const
    {
        return  mViewLowest ? mLowLimit : mHighLimit;
    }

    STAmount const& getLimitPeer () const
    {
        return !mViewLowest ? mLowLimit : mHighLimit;
    }

    Rate const&
    getQualityIn () const
    {
        return mViewLowest ? lowQualityIn_ : highQualityIn_;
    }

    Rate const&
    getQualityOut () const
    {
        return mViewLowest ? lowQualityOut_ : highQualityOut_;
    }

    Json::Value getJson (int);

    STAmount precision;

private:
    std::shared_ptr<SLE const> sle_;

    bool                            mViewLowest;

    std::uint32_t                   mFlags;

    STAmount const&                 mLowLimit;
    STAmount const&                 mHighLimit;

    AccountID const&                  mLowID;
    AccountID const&                  mHighID;

    Rate lowQualityIn_;
    Rate lowQualityOut_;
    Rate highQualityIn_;
    Rate highQualityOut_;

    STAmount                        mBalance;
};

std::vector <MTChainState::pointer>
getMTChainStateItems (AccountID const& accountID,
    ReadView const& view);

} //

#endif
