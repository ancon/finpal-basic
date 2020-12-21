//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_CONSENSUS_RCLCXPOSITION_H_INCLUDED
#define MTCHAIN_APP_CONSENSUS_RCLCXPOSITION_H_INCLUDED

#include <mtchain/app/ledger/LedgerProposal.h>
#include <mtchain/json/json_value.h>
#include <mtchain/basics/chrono.h>
#include <mtchain/protocol/UintTypes.h>

namespace mtchain {

// A position taken during a consensus round
// As seen by the RCL consensus process
class RCLCxPos
{

public:

    static std::uint32_t constexpr seqInitial = 0;
    static std::uint32_t constexpr seqLeave = 0xffffffff;

    RCLCxPos (LedgerProposal const& prop) :
        proposal_ (prop)
    { }

    std::uint32_t getSequence() const
    {
        return proposal_.getProposeSeq();
    }

    NetClock::time_point getCloseTime () const
    {
        return proposal_.getCloseTime();
    }

    NetClock::time_point getSeenTime() const
    {
        return proposal_.getSeenTime();
    }

    bool isStale (NetClock::time_point lastValid) const
    {
        return getSeenTime() < lastValid;
    }

    NodeID const& getNodeID() const
    {
        return proposal_.getPeerID();
    }

    LedgerHash const& getPosition() const
    {
        return proposal_.getCurrentHash();
    }

    LedgerHash const& getPrevLedger() const
    {
        return proposal_.getPrevLedger();
    }

    bool changePosition (
        LedgerHash const& position,
        NetClock::time_point closeTime,
        NetClock::time_point now)
    {
        return proposal_.changePosition (position, closeTime, now);
    }

    bool bowOut (NetClock::time_point now)
    {
        if (isBowOut ())
            return false;

        proposal_.bowOut (now);
        return true;
    }

    Json::Value getJson() const
    {
        return proposal_.getJson();
    }

    bool isInitial () const
    {
        return getSequence() == seqInitial;
    }

    bool isBowOut() const
    {
        return getSequence() == seqLeave;
    }

    // These three functions will be removed. New code
    // should use getPosition, getSequence and getNodeID
    LedgerHash const& getCurrentHash() const
    {
        return getPosition();
    }
    NodeID const& getPeerID() const
    {
        return getNodeID();
    }
    std::uint32_t getProposeSeq() const
    {
        return getSequence();
    }

    LedgerProposal const& peek() const
    {
        return proposal_;
    }

    LedgerProposal& peek()
    {
        return proposal_;
    }

protected:

    LedgerProposal proposal_;

};

}
#endif
