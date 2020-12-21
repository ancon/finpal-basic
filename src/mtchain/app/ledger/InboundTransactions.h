//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_INBOUNDTRANSACTIONS_H_INCLUDED
#define MTCHAIN_APP_LEDGER_INBOUNDTRANSACTIONS_H_INCLUDED

#include <mtchain/overlay/Peer.h>
#include <mtchain/shamap/SHAMap.h>
#include <mtchain/beast/clock/abstract_clock.h>
#include <mtchain/core/Stoppable.h>
#include <memory>

namespace mtchain {

class Application;

/** Manages the acquisition and lifetime of transaction sets.
*/

class InboundTransactions
{
public:
    using clock_type = beast::abstract_clock <std::chrono::steady_clock>;

    InboundTransactions() = default;
    InboundTransactions(InboundTransactions const&) = delete;
    InboundTransactions& operator=(InboundTransactions const&) = delete;

    virtual ~InboundTransactions() = 0;

    /** Retrieves a transaction set by hash
    */
    virtual std::shared_ptr <SHAMap> getSet (
        uint256 const& setHash,
        bool acquire) = 0;

    /** Gives data to an inbound transaction set
    */
    virtual void gotData (uint256 const& setHash,
        std::shared_ptr <Peer>,
        std::shared_ptr <protocol::TMLedgerData>) = 0;

    /** Gives set to the container
    */
    virtual void giveSet (uint256 const& setHash,
        std::shared_ptr <SHAMap> const& set,
        bool acquired) = 0;

    /** Informs the container if a new consensus round
    */
    virtual void newRound (std::uint32_t seq) = 0;

    virtual Json::Value getInfo() = 0;

    virtual void onStop() = 0;
};

std::unique_ptr <InboundTransactions>
make_InboundTransactions (
    Application& app,
    InboundTransactions::clock_type& clock,
    Stoppable& parent,
    beast::insight::Collector::ptr const& collector,
    std::function
        <void (std::shared_ptr <SHAMap> const&,
            bool)> gotSet);


} //

#endif
