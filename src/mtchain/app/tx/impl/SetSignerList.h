//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_SETSIGNERLIST_H_INCLUDED
#define MTCHAIN_TX_SETSIGNERLIST_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/app/tx/impl/SignerEntries.h>
#include <mtchain/protocol/STObject.h>
#include <mtchain/protocol/STArray.h>
#include <mtchain/protocol/STTx.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/basics/Log.h>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace mtchain {

/**
See the README.md for an overview of the SetSignerList transaction that
this class implements.
*/
class SetSignerList : public Transactor
{
private:
    // Values determined during preCompute for use later.
    enum Operation {unknown, set, destroy};
    Operation do_ {unknown};
    std::uint32_t quorum_ {0};
    std::vector<SignerEntries::SignerEntry> signers_;

public:
    SetSignerList (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    bool
    affectsSubsequentTransactionAuth(STTx const& tx)
    {
        return true;
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    TER doApply () override;
    void preCompute() override;

private:
    static
    std::tuple<TER, std::uint32_t,
        std::vector<SignerEntries::SignerEntry>,
            Operation>
    determineOperation(STTx const& tx,
        ApplyFlags flags, beast::Journal j);

    static
    TER validateQuorumAndSignerEntries (
        std::uint32_t quorum,
            std::vector<SignerEntries::SignerEntry> const& signers,
                AccountID const& account,
                    beast::Journal j);

    TER replaceSignerList ();
    TER destroySignerList ();

    TER removeSignersFromLedger (Keylet const& accountKeylet,
        Keylet const& ownerDirKeylet, Keylet const& signerListKeylet);
    void writeSignersToSLE (SLE::pointer const& ledgerEntry) const;

    static int ownerCountDelta (std::size_t entryCount);
};

} //

#endif
