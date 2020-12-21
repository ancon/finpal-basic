//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_NODEDIRECTORY_H_INCLUDED
#define MTCHAIN_APP_PATHS_NODEDIRECTORY_H_INCLUDED

#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/STLedgerEntry.h>
#include <mtchain/ledger/ApplyView.h>

namespace mtchain {

// VFALCO TODO de-inline these function definitions

class NodeDirectory
{
public:
    // Current directory - the last 64 bits of this are the quality.
    uint256 current;

    // Start of the next order book - one past the worst quality possible
    // for the current order book.
    uint256 next;

    // TODO(tom): directory.current and directory.next should be of type
    // Directory.

    bool advanceNeeded;  // Need to advance directory.
    bool restartNeeded;  // Need to restart directory.

    SLE::pointer ledgerEntry;

    void restart (bool multiQuality)
    {
        if (multiQuality)
            current = 0;             // Restart book searching.
        else
            restartNeeded  = true;   // Restart at same quality.
    }

    bool initialize (Book const& book, ApplyView& view)
    {
        if (current != zero)
            return false;

        current.copyFrom (getBookBase (book));
        next.copyFrom (getQualityNext (current));

        // TODO(tom): it seems impossible that any actual offers with
        // quality == 0 could occur - we should disallow them, and clear
        // directory.ledgerEntry without the database call in the next line.
        ledgerEntry = view.peek (keylet::page(current));

        // Advance, if didn't find it. Normal not to be unable to lookup
        // firstdirectory. Maybe even skip this lookup.
        advanceNeeded  = ! ledgerEntry;
        restartNeeded  = false;

        // Associated vars are dirty, if found it.
        return bool(ledgerEntry);
    }

    enum Advance
    {
        NO_ADVANCE,
        NEW_QUALITY,
        END_ADVANCE
    };

    /** Advance to the next quality directory in the order book. */
    // VFALCO Consider renaming this to `nextQuality` or something
    Advance
    advance (ApplyView& view)
    {
        if (!(advanceNeeded || restartNeeded))
            return NO_ADVANCE;

        // Get next quality.
        // The Merkel radix tree is ordered by key so we can go to the next
        // quality in O(1).
        if (advanceNeeded)
        {
            auto const opt =
                view.succ (current, next);
            current = opt ? *opt : uint256{};
        }
        advanceNeeded  = false;
        restartNeeded  = false;

        if (current == zero)
            return END_ADVANCE;

        ledgerEntry = view.peek (keylet::page(current));
        return NEW_QUALITY;
    }
};

} //

#endif
