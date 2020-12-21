//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/SignerEntries.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/STObject.h>
#include <mtchain/protocol/STArray.h>
#include <cstdint>

namespace mtchain {

std::pair<std::vector<SignerEntries::SignerEntry>, TER>
SignerEntries::deserialize (
    STObject const& obj, beast::Journal journal, std::string const& annotation)
{
    std::pair<std::vector<SignerEntry>, TER> s;

    if (!obj.isFieldPresent (sfSignerEntries))
    {
        JLOG(journal.trace()) <<
            "Malformed " << annotation << ": Need signer entry array.";
        s.second = temMALFORMED;
        return s;
    }

    auto& accountVec = s.first;
    accountVec.reserve (STTx::maxMultiSigners);

    STArray const& sEntries (obj.getFieldArray (sfSignerEntries));
    for (STObject const& sEntry : sEntries)
    {
        // Validate the SignerEntry.
        if (sEntry.getFName () != sfSignerEntry)
        {
            JLOG(journal.trace()) <<
                "Malformed " << annotation << ": Expected SignerEntry.";
            s.second = temMALFORMED;
            return s;
        }

        // Extract SignerEntry fields.
        AccountID const account = sEntry.getAccountID (sfAccount);
        std::uint16_t const weight = sEntry.getFieldU16 (sfSignerWeight);
        accountVec.emplace_back (account, weight);
    }

    s.second = tesSUCCESS;
    return s;
}

} //
