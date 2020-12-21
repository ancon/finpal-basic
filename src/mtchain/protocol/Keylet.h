//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_PROTOCOL_KEYLET_H_INCLUDED
#define MTCHAIN_PROTOCOL_KEYLET_H_INCLUDED

#include <mtchain/protocol/LedgerFormats.h>
#include <mtchain/basics/base_uint.h>

namespace mtchain {

class STLedgerEntry;

/** A pair of SHAMap key and LedgerEntryType.

    A Keylet identifies both a key in the state map
    and its ledger entry type.

    @note Keylet is a portmanteau of the words key
          and LET, an acronym for LedgerEntryType.
*/
struct Keylet
{
    LedgerEntryType type;
    uint256 key;

    Keylet (LedgerEntryType type_,
            uint256 const& key_)
        : type(type_)
        , key(key_)
    {
    }

    /** Returns true if the SLE matches the type */
    bool
    check (STLedgerEntry const&) const;
};

}

#endif
