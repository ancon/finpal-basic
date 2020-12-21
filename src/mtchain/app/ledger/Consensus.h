//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_CONSENSUS_H_INCLUDED
#define MTCHAIN_APP_LEDGER_CONSENSUS_H_INCLUDED

#include <mtchain/app/ledger/LedgerConsensus.h>
#include <mtchain/app/ledger/LedgerMaster.h>
#include <mtchain/app/ledger/InboundTransactions.h>
#include <mtchain/app/consensus/RCLCxTraits.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>

#include <memory>

namespace mtchain {

class LocalTxs;

/** Implements the consensus process and provides inter-round state. */
class Consensus
{
public:
    using Proposals = hash_map <NodeID, std::deque<LedgerProposal::pointer>>;

    virtual
    ~Consensus () = default;

    /** Returns whether we are issuing proposals currently. */
    virtual
    bool
    isProposing () const = 0;

    /** Returns whether we are issuing validations currently. */
    virtual
    bool
    isValidating () const = 0;

    /** Returns the number of unique proposers we observed for the LCL. */
    virtual
    int
    getLastCloseProposers () const = 0;

    /** Returns the time (in milliseconds) that the last close took. */
    virtual
    std::chrono::milliseconds
    getLastCloseDuration () const = 0;

    /** Called to create a LedgerConsensus instance */
    virtual
    std::shared_ptr<LedgerConsensus<RCLCxTraits>>
    makeLedgerConsensus (
        Application& app,
        InboundTransactions& inboundTransactions,
        LedgerMaster& ledgerMaster,
        LocalTxs& localTxs) = 0;

    /** Called when a new round of consensus is about to begin */
    virtual
    void
    startRound (
        LedgerConsensus<RCLCxTraits>& consensus,
        LedgerHash const &prevLCLHash,
        std::shared_ptr<Ledger const> const& previousLedger,
        NetClock::time_point closeTime) = 0;

    /** Specified the network time when the last ledger closed */
    virtual
    void
    setLastCloseTime (NetClock::time_point t) = 0;

    virtual
    void
    storeProposal (
        LedgerProposal::ref proposal,
        NodeID const& nodeID) = 0;
};

std::unique_ptr<Consensus>
make_Consensus (Config const& config, Logs& logs);

}

#endif
