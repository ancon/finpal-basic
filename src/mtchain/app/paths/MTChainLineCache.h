//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_MTCHAINLINECACHE_H_INCLUDED
#define MTCHAIN_APP_PATHS_MTCHAINLINECACHE_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/app/paths/MTChainState.h>
#include <mtchain/basics/hardened_hash.h>
#include <cstddef>
#include <memory>
#include <mutex>
#include <vector>

namespace mtchain {

// Used by Pathfinder
class MTChainLineCache
{
public:
    explicit
    MTChainLineCache (
        std::shared_ptr <ReadView const> const& l);

    std::shared_ptr <ReadView const> const&
    getLedger () const
    {
        return mLedger;
    }

    std::vector<MTChainState::pointer> const&
    getMTChainLines (AccountID const& accountID);

private:
    std::mutex mLock;

    mtchain::hardened_hash<> hasher_;
    std::shared_ptr <ReadView const> mLedger;

    struct AccountKey
    {
        AccountID account_;
        std::size_t hash_value_;

        AccountKey (AccountID const& account, std::size_t hash)
            : account_ (account)
            , hash_value_ (hash)
        { }

        AccountKey (AccountKey const& other) = default;

        AccountKey&
        operator=(AccountKey const& other) = default;

        bool operator== (AccountKey const& lhs) const
        {
            return hash_value_ == lhs.hash_value_ && account_ == lhs.account_;
        }

        std::size_t
        get_hash () const
        {
            return hash_value_;
        }

        struct Hash
        {
            std::size_t
            operator () (AccountKey const& key) const noexcept
            {
                return key.get_hash ();
            }
        };
    };

    hash_map <
        AccountKey,
        std::vector <MTChainState::pointer>,
        AccountKey::Hash> lines_;
};

} //

#endif
