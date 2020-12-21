//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_IMPL_SIGNER_ENTRIES_H_INCLUDED
#define MTCHAIN_TX_IMPL_SIGNER_ENTRIES_H_INCLUDED

#include <mtchain/protocol/STTx.h>      // STTx::maxMultiSigners
#include <mtchain/protocol/UintTypes.h> // AccountID
#include <mtchain/protocol/TER.h>       // temMALFORMED
#include <mtchain/beast/utility/Journal.h>     // beast::Journal

namespace mtchain {

// Forward declarations
class STObject;

// Support for SignerEntries that is needed by a few Transactors
class SignerEntries
{
public:
    struct SignerEntry
    {
        AccountID account;
        std::uint16_t weight;

        SignerEntry (AccountID const& inAccount, std::uint16_t inWeight)
        : account (inAccount)
        , weight (inWeight)
        { }

        // For sorting to look for duplicate accounts
        friend bool operator< (SignerEntry const& lhs, SignerEntry const& rhs)
        {
            return lhs.account < rhs.account;
        }

        friend bool operator== (SignerEntry const& lhs, SignerEntry const& rhs)
        {
            return lhs.account == rhs.account;
        }
    };

    // Deserialize a SignerEntries array from the network or from the ledger.
    static
    std::pair<std::vector<SignerEntry>, TER>
    deserialize (
        STObject const& obj,
        beast::Journal journal,
        std::string const& annotation);
};

} //

#endif // MTCHAIN_TX_IMPL_SIGNER_ENTRIES_H_INCLUDED
