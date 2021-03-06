//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_CANONICALTXSET_H_INCLUDED
#define MTCHAIN_APP_MISC_CANONICALTXSET_H_INCLUDED

#include <mtchain/protocol/MTChainLedgerHash.h>
#include <mtchain/protocol/STTx.h>

namespace mtchain {

/** Holds transactions which were deferred to the next pass of consensus.

    "Canonical" refers to the order in which transactions are applied.

    - Puts transactions from the same account in sequence order

*/
// VFALCO TODO rename to SortedTxSet
class CanonicalTXSet
{
private:
    class Key
    {
    public:
        Key (uint256 const& account, std::uint32_t seq, uint256 const& id)
            : mAccount (account)
            , mTXid (id)
            , mSeq (seq)
        {
        }

        bool operator<  (Key const& rhs) const;
        bool operator>  (Key const& rhs) const;
        bool operator<= (Key const& rhs) const;
        bool operator>= (Key const& rhs) const;

        bool operator== (Key const& rhs) const
        {
            return mTXid == rhs.mTXid;
        }
        bool operator!= (Key const& rhs) const
        {
            return mTXid != rhs.mTXid;
        }

        uint256 const& getTXID () const
        {
            return mTXid;
        }

    private:
        uint256 mAccount;
        uint256 mTXid;
        std::uint32_t mSeq;
    };

    // Calculate the salted key for the given account
    uint256 accountKey (AccountID const& account);

public:
    using iterator = std::map <Key, std::shared_ptr<STTx const>>::iterator;
    using const_iterator = std::map <Key, std::shared_ptr<STTx const>>::const_iterator;

public:
    explicit CanonicalTXSet (LedgerHash const& saltHash)
        : mSetHash (saltHash)
    {
    }

    void insert (std::shared_ptr<STTx const> const& txn);

    std::vector<std::shared_ptr<STTx const>>
    prune(AccountID const& account, std::uint32_t const seq);

    // VFALCO TODO remove this function
    void reset (LedgerHash const& saltHash)
    {
        mSetHash = saltHash;

        mMap.clear ();
    }

    iterator erase (iterator const& it);

    iterator begin ()
    {
        return mMap.begin ();
    }
    iterator end ()
    {
        return mMap.end ();
    }
    const_iterator begin ()  const
    {
        return mMap.begin ();
    }
    const_iterator end () const
    {
        return mMap.end ();
    }
    size_t size () const
    {
        return mMap.size ();
    }
    bool empty () const
    {
        return mMap.empty ();
    }

private:
    // Used to salt the accounts so people can't mine for low account numbers
    uint256 mSetHash;

    std::map <Key, std::shared_ptr<STTx const>> mMap;
};

} //

#endif
