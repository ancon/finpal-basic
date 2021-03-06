//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_LEDGERPROPOSAL_H_INCLUDED
#define MTCHAIN_APP_LEDGER_LEDGERPROPOSAL_H_INCLUDED

#include <mtchain/basics/CountedObject.h>
#include <mtchain/basics/base_uint.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/HashPrefix.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/protocol/SecretKey.h>
#include <mtchain/beast/hash/hash_append.h>
#include <chrono>
#include <cstdint>
#include <string>

namespace mtchain {

class LedgerProposal
    : public CountedObject <LedgerProposal>
{
private:
    // A peer initial joins the consensus process
    static std::uint32_t const seqJoin = 0;

    // A peer wants to bow out and leave the consensus process
    static std::uint32_t const seqLeave = 0xffffffff;

public:
    static char const* getCountedObjectName () { return "LedgerProposal"; }

    using pointer = std::shared_ptr<LedgerProposal>;
    using ref = const pointer&;

    // proposal from peer
    LedgerProposal (
        uint256 const& prevLgr,
        std::uint32_t proposeSeq,
        uint256 const& propose,
        NetClock::time_point closeTime,
        NetClock::time_point now,
        PublicKey const& publicKey,
        NodeID const& nodeID,
        Slice const& signature,
        uint256 const& suppress);

    // Our own proposal:
    LedgerProposal (
        uint256 const& prevLedger,
        uint256 const& position,
        NetClock::time_point closeTime,
        NetClock::time_point now);

    uint256 getSigningHash () const;
    bool checkSign () const;

    NodeID const& getPeerID () const
    {
        return mPeerID;
    }
    uint256 const& getCurrentHash () const
    {
        return mCurrentHash;
    }
    uint256 const& getPrevLedger () const
    {
        return mPreviousLedger;
    }
    PublicKey const& getPublicKey () const
    {
        return publicKey_;
    }
    uint256 const& getSuppressionID () const
    {
        return mSuppression;
    }
    std::uint32_t getProposeSeq () const
    {
        return mProposeSeq;
    }
    NetClock::time_point getCloseTime () const
    {
        return mCloseTime;
    }
    NetClock::time_point getSeenTime () const
    {
        return mTime;
    }
    Blob const& getSignature () const
    {
        return signature_;
    }
    bool isInitial () const
    {
        return mProposeSeq == seqJoin;
    }
    bool isBowOut () const
    {
        return mProposeSeq == seqLeave;
    }

    bool isStale (NetClock::time_point cutoff) const
    {
        return mTime <= cutoff;
    }

    bool changePosition (
        uint256 const& newPosition,
        NetClock::time_point newCloseTime,
        NetClock::time_point now);
    void bowOut (NetClock::time_point now);
    Json::Value getJson () const;

private:
    template <class Hasher>
    void
    hash_append (Hasher& h) const
    {
        using beast::hash_append;
        hash_append(h, HashPrefix::proposal);
        hash_append(h, std::uint32_t(mProposeSeq));
        hash_append(h, mCloseTime);
        hash_append(h, mPreviousLedger);
        hash_append(h, mCurrentHash);
    }

    uint256 mPreviousLedger, mCurrentHash, mSuppression;
    NetClock::time_point mCloseTime;
    std::uint32_t mProposeSeq;

    PublicKey publicKey_;
    NodeID mPeerID;
    Blob signature_;

    NetClock::time_point mTime;
};

/** Calculate a unique identifier for a signed proposal.

    The identifier is based on all the fields that contribute to the signature,
    as well as the signature itself. The "last closed ledger" field may be
    omitted, but the signer will compute the signature as if this field was
    present. Recipients of the proposal will inject the last closed ledger in
    order to validate the signature. If the last closed ledger is left out, then
    it is considered as all zeroes for the purposes of signing.
*/
uint256 proposalUniqueId (
        uint256 const& proposeHash,
        uint256 const& previousLedger,
        std::uint32_t proposeSeq,
        NetClock::time_point closeTime,
        Slice const& publicKey,
        Slice const& signature);

} //

#endif
