//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_HASHROUTER_H_INCLUDED
#define MTCHAIN_APP_MISC_HASHROUTER_H_INCLUDED

#include <mtchain/basics/base_uint.h>
#include <mtchain/basics/chrono.h>
#include <mtchain/basics/CountedObject.h>
#include <mtchain/basics/UnorderedContainers.h>
#include <mtchain/beast/container/aged_unordered_map.h>
#include <boost/optional.hpp>

namespace mtchain {

// VFALCO NOTE Are these the flags?? Why aren't we using a packed struct?
// VFALCO TODO convert these macros to int constants
// VFALCO NOTE How can both bad and good be set on a hash?
#define SF_BAD          0x02    // Temporarily bad
#define SF_SAVED        0x04
#define SF_RETRY        0x08    // Transaction can be retried
#define SF_TRUSTED      0x10    // comes from trusted source
// Private flags, used internally in apply.cpp.
// Do not attempt to read, set, or reuse.
#define SF_PRIVATE1     0x0100
#define SF_PRIVATE2     0x0200
#define SF_PRIVATE3     0x0400
#define SF_PRIVATE4     0x0800
#define SF_PRIVATE5     0x1000
#define SF_PRIVATE6     0x2000

/** Routing table for objects identified by hash.

    This table keeps track of which hashes have been received by which peers.
    It is used to manage the routing and broadcasting of messages in the peer
    to peer overlay.
*/
class HashRouter
{
public:
    // The type here *MUST* match the type of Peer::id_t
    using PeerShortID = std::uint32_t;

private:
    /** An entry in the routing table.
    */
    class Entry : public CountedObject <Entry>
    {
    public:
        static char const* getCountedObjectName () { return "HashRouterEntry"; }

        Entry ()
            : flags_ (0)
        {
        }

        void addPeer (PeerShortID peer)
        {
            if (peer != 0)
                peers_.insert (peer);
        }

        int getFlags (void) const
        {
            return flags_;
        }

        void setFlags (int flagsToSet)
        {
            flags_ |= flagsToSet;
        }

        /** Return set of peers we've relayed to and reset tracking */
        std::set<PeerShortID> releasePeerSet()
        {
            return std::move(peers_);
        }

        /** Determines if this item should be relayed.

            Checks whether the item has been recently relayed.
            If it has, return false. If it has not, update the
            last relay timestamp and return true.
        */
        bool shouldRelay (Stopwatch::time_point const& now,
            std::chrono::seconds holdTime)
        {
            if (relayed_ && *relayed_ + holdTime > now)
                return false;
            relayed_.emplace(now);
            return true;
        }

    private:
        int flags_;
        std::set <PeerShortID> peers_;
        // This could be generalized to a map, if more
        // than one flag needs to expire independently.
        boost::optional<Stopwatch::time_point> relayed_;
    };

public:
    static inline std::chrono::seconds getDefaultHoldTime ()
    {
        using namespace std::chrono;

        return 300s;
    }

    HashRouter (Stopwatch& clock, std::chrono::seconds entryHoldTimeInSeconds)
        : suppressionMap_(clock)
        , holdTime_ (entryHoldTimeInSeconds)
    {
    }

    HashRouter& operator= (HashRouter const&) = delete;

    virtual ~HashRouter() = default;

    // VFALCO TODO Replace "Supression" terminology with something more
    // semantically meaningful.
    void addSuppression(uint256 const& key);

    bool addSuppressionPeer (uint256 const& key, PeerShortID peer);

    bool addSuppressionPeer (uint256 const& key, PeerShortID peer,
                             int& flags);

    /** Set the flags on a hash.

        @return `true` if the flags were changed. `false` if unchanged.
    */
    bool setFlags (uint256 const& key, int flags);

    int getFlags (uint256 const& key);

    /** Determines whether the hashed item should be relayed.

        Effects:

            If the item should be relayed, this function will not
            return `true` again until the hold time has expired.
            The internal set of peers will also be reset.

        @return A `boost::optional` set of peers which do not need to be
            relayed to. If the result is uninitialized, the item should
            _not_ be relayed.
    */
    boost::optional<std::set<PeerShortID>> shouldRelay(uint256 const& key);

private:
    // pair.second indicates whether the entry was created
    std::pair<Entry&, bool> emplace (uint256 const&);

    std::mutex mutable mutex_;

    // Stores all suppressed hashes and their expiration time
    beast::aged_unordered_map<uint256, Entry, Stopwatch::clock_type,
        hardened_hash<strong_hash>> suppressionMap_;

    std::chrono::seconds const holdTime_;
};

} //

#endif
