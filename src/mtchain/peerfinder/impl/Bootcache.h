//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PEERFINDER_BOOTCACHE_H_INCLUDED
#define MTCHAIN_PEERFINDER_BOOTCACHE_H_INCLUDED

#include <mtchain/peerfinder/PeerfinderManager.h>
#include <mtchain/peerfinder/impl/Store.h>
#include <mtchain/beast/utility/Journal.h>
#include <mtchain/beast/utility/PropertyStream.h>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/iterator/transform_iterator.hpp>

namespace mtchain {
namespace PeerFinder {

/** Stores IP addresses useful for gaining initial connections.

    This is one of the caches that is consulted when additional outgoing
    connections are needed. Along with the address, each entry has this
    additional metadata:

    Valence
        A signed integer which represents the number of successful
        consecutive connection attempts when positive, and the number of
        failed consecutive connection attempts when negative.

    When choosing addresses from the boot cache for the purpose of
    establishing outgoing connections, addresses are ranked in decreasing
    order of high uptime, with valence as the tie breaker.
*/
class Bootcache
{
private:
    class Entry
    {
    public:
        Entry (int valence)
            : m_valence (valence)
        {
        }

        int& valence ()
        {
            return m_valence;
        }

        int valence () const
        {
            return m_valence;
        }

        friend bool operator< (Entry const& lhs, Entry const& rhs)
        {
            if (lhs.valence() > rhs.valence())
                return true;
            return false;
        }

    private:
        int m_valence;
    };

    using left_t = boost::bimaps::unordered_set_of <beast::IP::Endpoint>;
    using right_t = boost::bimaps::multiset_of <Entry>;
    using map_type = boost::bimap <left_t, right_t>;
    using value_type = map_type::value_type;

    struct Transform : std::unary_function <
        map_type::right_map::const_iterator::value_type const&,
            beast::IP::Endpoint const&>
    {
        beast::IP::Endpoint const& operator() (
            map_type::right_map::
                const_iterator::value_type const& v) const
        {
            return v.get_left();
        }
    };

private:
    map_type m_map;

    Store& m_store;
    clock_type& m_clock;
    beast::Journal m_journal;

    // Time after which we can update the database again
    clock_type::time_point m_whenUpdate;

    // Set to true when a database update is needed
    bool m_needsUpdate;

public:
    using iterator = boost::transform_iterator <Transform,
        map_type::right_map::const_iterator>;

    using const_iterator = iterator;

    Bootcache (
        Store& store,
        clock_type& clock,
        beast::Journal journal);

    ~Bootcache ();

    /** Returns `true` if the cache is empty. */
    bool empty() const;

    /** Returns the number of entries in the cache. */
    map_type::size_type size() const;

    /** IP::Endpoint iterators that traverse in decreasing valence. */
    /** @{ */
    const_iterator begin() const;
    const_iterator cbegin() const;
    const_iterator end() const;
    const_iterator cend() const;
    void clear();
    /** @} */

    /** Load the persisted data from the Store into the container. */
    void load ();

    /** Add the address to the cache. */
    bool insert (beast::IP::Endpoint const& endpoint);

    /** Called when an outbound connection handshake completes. */
    void on_success (beast::IP::Endpoint const& endpoint);

    /** Called when an outbound connection attempt fails to handshake. */
    void on_failure (beast::IP::Endpoint const& endpoint);

    /** Stores the cache in the persistent database on a timer. */
    void periodicActivity ();

    /** Write the cache state to the property stream. */
    void onWrite (beast::PropertyStream::Map& map);

private:
    void prune ();
    void update ();
    void checkUpdate ();
    void flagForUpdate ();
};

}
}

#endif