//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_OVERLAY_PEER_H_INCLUDED
#define MTCHAIN_OVERLAY_PEER_H_INCLUDED

#include <mtchain/overlay/Message.h>
#include <mtchain/basics/base_uint.h>
#include <mtchain/json/json_value.h>
#include <mtchain/protocol/PublicKey.h>
#include <mtchain/beast/net/IPEndpoint.h>

namespace mtchain {

namespace Resource {
class Charge;
}

/** Represents a peer connection in the overlay. */
class Peer
{
public:
    using ptr = std::shared_ptr<Peer>;

    /** Uniquely identifies a peer.
        This can be stored in tables to find the peer later. Callers
        can discover if the peer is no longer connected and make
        adjustments as needed.
    */
    using id_t = std::uint32_t;

    virtual ~Peer() = default;

    //
    // Network
    //

    virtual
    void
    send (Message::pointer const& m) = 0;

    virtual
    beast::IP::Endpoint
    getRemoteAddress() const = 0;

    /** Adjust this peer's load balance based on the type of load imposed. */
    virtual
    void
    charge (Resource::Charge const& fee) = 0;

    //
    // Identity
    //

    virtual
    id_t
    id() const = 0;

    /** Returns `true` if this connection is a member of the cluster. */
    virtual
    bool
    cluster() const = 0;

    virtual
    bool
    isHighLatency() const = 0;

    virtual
    int
    getScore (bool) const = 0;

    virtual
    PublicKey const&
    getNodePublic() const = 0;

    virtual
    Json::Value json() = 0;

    //
    // Ledger
    //

    virtual uint256 const& getClosedLedgerHash () const = 0;
    virtual bool hasLedger (uint256 const& hash, std::uint32_t seq) const = 0;
    virtual void ledgerRange (std::uint32_t& minSeq, std::uint32_t& maxSeq) const = 0;
    virtual bool hasTxSet (uint256 const& hash) const = 0;
    virtual void cycleStatus () = 0;
    virtual bool supportsVersion (int version) = 0;
    virtual bool hasRange (std::uint32_t uMin, std::uint32_t uMax) = 0;
};

}

#endif
