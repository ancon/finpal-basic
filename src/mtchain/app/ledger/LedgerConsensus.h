//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_LEDGERCONSENSUS_H_INCLUDED
#define MTCHAIN_APP_LEDGER_LEDGERCONSENSUS_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/app/ledger/LedgerProposal.h>
#include <mtchain/app/ledger/InboundTransactions.h>
#include <mtchain/app/consensus/RCLCxTraits.h>
#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/CanonicalTXSet.h>
#include <mtchain/app/misc/FeeVote.h>
#include <mtchain/json/json_value.h>
#include <mtchain/overlay/Peer.h>
#include <mtchain/protocol/MTChainLedgerHash.h>
#include <chrono>

namespace mtchain {

/** Manager for achieving consensus on the next ledger.
*/
template <class Traits>
class LedgerConsensus : public Traits
{
public:

    using typename Traits::Time_t;
    using typename Traits::Pos_t;
    using typename Traits::TxSet_t;
    using typename Traits::Tx_t;
    using typename Traits::LgrID_t;
    using typename Traits::TxID_t;
    using typename Traits::TxSetID_t;
    using typename Traits::NodeID_t;

    virtual ~LedgerConsensus() = default;

    virtual Json::Value getJson (bool full) = 0;

    virtual LgrID_t getLCL () = 0;

    virtual void gotMap (TxSet_t const& map) = 0;

    virtual void timerEntry () = 0;

    virtual bool peerPosition (Pos_t const& position) = 0;

    virtual PublicKey const& getValidationPublicKey () const = 0;

    virtual void setValidationKeys (
        SecretKey const& valSecret, PublicKey const& valPublic) = 0;

    virtual void startRound (
        LgrID_t const& prevLCLHash,
        std::shared_ptr<Ledger const> const& prevLedger,
        Time_t closeTime,
        int previousProposers,
        std::chrono::milliseconds previousConvergeTime) = 0;

    /** Simulate the consensus process without any network traffic.

        The end result, is that consensus begins and completes as if everyone
        had agreed with whatever we propose.

        This function is only called from the rpc "ledger_accept" path with the
        server in standalone mode and SHOULD NOT be used during the normal
        consensus process.
    */
    virtual void simulate (
        boost::optional<std::chrono::milliseconds> consensusDelay) = 0;
};

} //

#endif
