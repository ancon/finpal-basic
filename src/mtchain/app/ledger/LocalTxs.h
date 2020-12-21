//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_LEDGER_LOCALTXS_H_INCLUDED
#define MTCHAIN_APP_LEDGER_LOCALTXS_H_INCLUDED

#include <mtchain/app/misc/CanonicalTXSet.h>
#include <mtchain/ledger/ReadView.h>
#include <memory>

namespace mtchain {

// Track transactions issued by local clients
// Ensure we always apply them to our open ledger
// Hold them until we see them in a fully-validated ledger

class LocalTxs
{
public:
    virtual ~LocalTxs () = default;

    // Add a new local transaction
    virtual void push_back (LedgerIndex index, std::shared_ptr<STTx const> const& txn) = 0;

    // Return the set of local transactions to a new open ledger
    virtual CanonicalTXSet getTxSet () = 0;

    // Remove obsolete transactions based on a new fully-valid ledger
    virtual void sweep (ReadView const& view) = 0;

    virtual std::size_t size () = 0;
};

std::unique_ptr<LocalTxs>
make_LocalTxs ();

} //

#endif
