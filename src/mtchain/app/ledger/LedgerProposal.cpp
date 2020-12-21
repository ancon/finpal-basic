//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerProposal.h>
#include <mtchain/protocol/digest.h>
#include <mtchain/core/Config.h>
#include <mtchain/protocol/JsonFields.h>
#include <mtchain/protocol/HashPrefix.h>
#include <mtchain/protocol/Serializer.h>

namespace mtchain {

// Used to construct received proposals
LedgerProposal::LedgerProposal (
        uint256 const& pLgr,
        std::uint32_t seq,
        uint256 const& tx,
        NetClock::time_point closeTime,
        NetClock::time_point now,
        PublicKey const& publicKey,
        NodeID const& nodeID,
        Slice const& signature,
        uint256 const& suppression)
    : mPreviousLedger (pLgr)
    , mCurrentHash (tx)
    , mSuppression (suppression)
    , mCloseTime (closeTime)
    , mProposeSeq (seq)
    , publicKey_ (publicKey)
    , mPeerID (nodeID)
    , mTime (now)
{
    signature_.resize (signature.size());
    std::memcpy(signature_.data(),
        signature.data(), signature.size());
}

// Used to construct local proposals
// CAUTION: publicKey_ not set
LedgerProposal::LedgerProposal (
        uint256 const& prevLgr,
        uint256 const& position,
        NetClock::time_point closeTime,
        NetClock::time_point now)
    : mPreviousLedger (prevLgr)
    , mCurrentHash (position)
    , mCloseTime (closeTime)
    , mProposeSeq (seqJoin)
    , mTime (now)
{
}

uint256 LedgerProposal::getSigningHash () const
{
    return sha512Half(
        HashPrefix::proposal,
        std::uint32_t(mProposeSeq),
        mCloseTime.time_since_epoch().count(),
        mPreviousLedger,
        mCurrentHash);
}

bool LedgerProposal::checkSign () const
{
    return verifyDigest (
        publicKey_,
        getSigningHash(),
        makeSlice (signature_),
        false);
}

bool LedgerProposal::changePosition (
    uint256 const& newPosition,
    NetClock::time_point closeTime,
    NetClock::time_point now)
{
    if (mProposeSeq == seqLeave)
        return false;

    mCurrentHash    = newPosition;
    mCloseTime      = closeTime;
    mTime           = now;
    ++mProposeSeq;
    return true;
}

void LedgerProposal::bowOut (NetClock::time_point now)
{
    mTime           = now;
    mProposeSeq     = seqLeave;
}

Json::Value LedgerProposal::getJson () const
{
    Json::Value ret = Json::objectValue;
    ret[jss::previous_ledger] = to_string (mPreviousLedger);

    if (mProposeSeq != seqLeave)
    {
        ret[jss::transaction_hash] = to_string (mCurrentHash);
        ret[jss::propose_seq] = mProposeSeq;
    }

    ret[jss::close_time] = mCloseTime.time_since_epoch().count();

    if (publicKey_.size())
        ret[jss::peer_id] =  toBase58 (
            TokenType::TOKEN_NODE_PUBLIC,
            publicKey_);

    return ret;
}

uint256 proposalUniqueId (
    uint256 const& proposeHash,
    uint256 const& previousLedger,
    std::uint32_t proposeSeq,
    NetClock::time_point closeTime,
    Slice const& publicKey,
    Slice const& signature)
{
    Serializer s (512);
    s.add256 (proposeHash);
    s.add256 (previousLedger);
    s.add32 (proposeSeq);
    s.add32 (closeTime.time_since_epoch().count());
    s.addVL (publicKey);
    s.addVL (signature);

    return s.getSHA512Half ();
}

} //
