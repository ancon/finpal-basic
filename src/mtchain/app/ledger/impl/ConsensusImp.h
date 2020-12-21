
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_IMPL_CONSENSUSIMP_H_INCLUDED
#define MTCHAIN_APP_LEDGER_IMPL_CONSENSUSIMP_H_INCLUDED

#include <BeastConfig.h>
#include <mtchain/app/ledger/Consensus.h>
#include <mtchain/app/ledger/LedgerConsensus.h>
#include <mtchain/app/misc/FeeVote.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/STValidation.h>
#include <mtchain/shamap/SHAMap.h>
#include <mtchain/beast/utility/Journal.h>

namespace mtchain {

/** Implements the consensus process and provides inter-round state. */
class ConsensusImp
    : public Consensus
{
public:
    ConsensusImp (FeeVote::Setup const& voteSetup, Logs& logs);

    ~ConsensusImp () = default;

    bool
    isProposing () const override;

    bool
    isValidating () const override;

    int
    getLastCloseProposers () const override;

    std::chrono::milliseconds
    getLastCloseDuration () const override;

    std::shared_ptr<LedgerConsensus<RCLCxTraits>>
    makeLedgerConsensus (
        Application& app,
        InboundTransactions& inboundTransactions,
        LedgerMaster& ledgerMaster,
        LocalTxs& localTxs) override;

    void
    startRound (
        LedgerConsensus<RCLCxTraits>& ledgerConsensus,
        LedgerHash const& prevLCLHash,
        std::shared_ptr<Ledger const> const& previousLedger,
        NetClock::time_point closeTime) override;

    void
    setLastCloseTime (NetClock::time_point t) override;

    void
    storeProposal (
        LedgerProposal::ref proposal,
        NodeID const& nodeID) override;

    void
    setProposing (bool p, bool v);

    STValidation::ref
    getLastValidation () const;

    void
    setLastValidation (STValidation::ref v);

    void
    newLCL (
        int proposers,
        std::chrono::milliseconds convergeTime);

    NetClock::time_point
    validationTimestamp (NetClock::time_point vt);

    NetClock::time_point
    getLastCloseTime () const;

    std::vector <RCLCxPos>
    getStoredProposals (uint256 const& previousLedger);

private:
    beast::Journal journal_;
    std::unique_ptr <FeeVote> feeVote_;

    bool proposing_;
    bool validating_;

    // A pointer to the last validation that we issued
    STValidation::pointer lastValidation_;

    // The number of proposers who participated in the last ledger close
    int lastCloseProposers_;

    // How long the last ledger close took, in milliseconds
    std::chrono::milliseconds lastCloseConvergeTook_;

    // The timestamp of the last validation we used, in network time. This is
    // only used for our own validations.
    NetClock::time_point lastValidationTimestamp_;

    // The last close time
    NetClock::time_point lastCloseTime_;

    Consensus::Proposals storedProposals_;

    // lock to protect storedProposals_
    std::mutex lock_;
};

}

#endif
