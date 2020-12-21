//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/LedgerTiming.h>
#include <mtchain/app/ledger/impl/ConsensusImp.h>
#include <mtchain/app/ledger/impl/LedgerConsensusImp.h>

namespace mtchain {

ConsensusImp::ConsensusImp (
        FeeVote::Setup const& voteSetup,
        Logs& logs)
    : journal_ (logs.journal("Consensus"))
    , feeVote_ (make_FeeVote (voteSetup,
        logs.journal("FeeVote")))
    , proposing_ (false)
    , validating_ (false)
    , lastCloseProposers_ (0)
    , lastCloseConvergeTook_ (LEDGER_IDLE_INTERVAL)
    , lastValidationTimestamp_ (0s)
    , lastCloseTime_ (0s)
{
}

bool
ConsensusImp::isProposing () const
{
    return proposing_;
}

bool
ConsensusImp::isValidating () const
{
    return validating_;
}

int
ConsensusImp::getLastCloseProposers () const
{
    return lastCloseProposers_;
}

std::chrono::milliseconds
ConsensusImp::getLastCloseDuration () const
{
    return lastCloseConvergeTook_;
}

std::shared_ptr<LedgerConsensus<RCLCxTraits>>
ConsensusImp::makeLedgerConsensus (
    Application& app,
    InboundTransactions& inboundTransactions,
    LedgerMaster& ledgerMaster,
    LocalTxs& localTxs)
{
    return make_LedgerConsensus (app, *this,
        inboundTransactions, localTxs, ledgerMaster, *feeVote_);
}

void
ConsensusImp::startRound (
    LedgerConsensus<RCLCxTraits>& consensus,
    LedgerHash const &prevLCLHash,
    std::shared_ptr<Ledger const> const& previousLedger,
    NetClock::time_point closeTime)
{
    consensus.startRound (
        prevLCLHash,
        previousLedger,
        closeTime,
        lastCloseProposers_,
        lastCloseConvergeTook_);
}

void
ConsensusImp::setProposing (bool p, bool v)
{
    proposing_ = p;
    validating_ = v;
}

STValidation::ref
ConsensusImp::getLastValidation () const
{
    return lastValidation_;
}

void
ConsensusImp::setLastValidation (STValidation::ref v)
{
    lastValidation_ = v;
}

void
ConsensusImp::newLCL (
    int proposers,
    std::chrono::milliseconds convergeTime)
{
    lastCloseProposers_ = proposers;
    lastCloseConvergeTook_ = convergeTime;
}

NetClock::time_point
ConsensusImp::validationTimestamp (NetClock::time_point vt)
{
    if (vt <= lastValidationTimestamp_)
        vt = lastValidationTimestamp_ + 1s;

    lastValidationTimestamp_ = vt;
    return vt;
}

NetClock::time_point
ConsensusImp::getLastCloseTime () const
{
    return lastCloseTime_;
}

void
ConsensusImp::setLastCloseTime (NetClock::time_point t)
{
    lastCloseTime_ = t;
}

void
ConsensusImp::storeProposal (
    LedgerProposal::ref proposal,
    NodeID const& nodeID)
{
    std::lock_guard <std::mutex> _(lock_);

    auto& props = storedProposals_[nodeID];

    if (props.size () >= 10)
        props.pop_front ();

    props.push_back (proposal);
}

std::vector <RCLCxPos>
ConsensusImp::getStoredProposals (uint256 const& prevLedger)
{

    std::vector <RCLCxPos> ret;

    {
        std::lock_guard <std::mutex> _(lock_);

        for (auto const& it : storedProposals_)
            for (auto const& prop : it.second)
                if (prop->getPrevLedger() == prevLedger)
                    ret.emplace_back (*prop);
    }

    return ret;
}

std::unique_ptr <Consensus>
make_Consensus (Config const& config, Logs& logs)
{
    return std::make_unique<ConsensusImp> (
        setup_FeeVote (config.section ("voting")),
        logs);
}

}
