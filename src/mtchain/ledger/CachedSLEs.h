//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_LEDGER_CACHEDSLES_H_INCLUDED
#define MTCHAIN_LEDGER_CACHEDSLES_H_INCLUDED

#include <mtchain/basics/chrono.h>
#include <mtchain/protocol/STLedgerEntry.h>
#include <mtchain/beast/container/aged_unordered_map.h>
#include <memory>
#include <mutex>

namespace mtchain {

/** Caches SLEs by their digest. */
class CachedSLEs
{
public:
    using digest_type = uint256;

    using value_type =
        std::shared_ptr<SLE const>;

    CachedSLEs (CachedSLEs const&) = delete;
    CachedSLEs& operator= (CachedSLEs const&) = delete;

    template <class Rep, class Period>
    CachedSLEs (std::chrono::duration<
        Rep, Period> const& timeToLive,
            Stopwatch& clock)
        : timeToLive_ (timeToLive)
        , map_ (clock)
    {
    }

    /** Discard expired entries.

        Needs to be called periodically.
    */
    void
    expire();

    /** Fetch an item from the cache.

        If the digest was not found, Handler
        will be called with this signature:

            std::shared_ptr<SLE const>(void)
    */
    template <class Handler>
    value_type
    fetch (digest_type const& digest,
        Handler const& h)
    {
        {
            std::lock_guard<
                std::mutex> lock(mutex_);
            auto iter =
                map_.find(digest);
            if (iter != map_.end())
            {
                ++hit_;
                map_.touch(iter);
                return iter->second;
            }
        }
        auto sle = h();
        if (! sle)
            return nullptr;
        std::lock_guard<
            std::mutex> lock(mutex_);
        ++miss_;
        auto const result =
            map_.emplace(
                digest, std::move(sle));
        if (! result.second)
            map_.touch(result.first);
        return  result.first->second;
    }

    /** Returns the fraction of cache hits. */
    double
    rate() const;

private:
    std::size_t hit_ = 0;
    std::size_t miss_ = 0;
    std::mutex mutable mutex_;
    Stopwatch::duration timeToLive_;
    beast::aged_unordered_map <digest_type,
        value_type, Stopwatch::clock_type,
            hardened_hash<strong_hash>> map_;
};

} //

#endif
