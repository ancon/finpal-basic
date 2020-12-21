//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/ledger/AcceptedLedger.h>
#include <mtchain/basics/Log.h>
#include <mtchain/basics/chrono.h>

namespace mtchain {

AcceptedLedger::AcceptedLedger (
    std::shared_ptr<ReadView const> const& ledger,
    AccountIDCache const& accountCache, Logs& logs)
    : mLedger (ledger)
{
    for (auto const& item : ledger->txs)
    {
        insert (std::make_shared<AcceptedLedgerTx>(
            ledger, item.first, item.second, accountCache, logs));
    }
}

void AcceptedLedger::insert (AcceptedLedgerTx::ref at)
{
    assert (mMap.find (at->getIndex ()) == mMap.end ());
    mMap.insert (std::make_pair (at->getIndex (), at));
}

AcceptedLedgerTx::pointer AcceptedLedger::getTxn (int i) const
{
    map_t::const_iterator it = mMap.find (i);

    if (it == mMap.end ())
        return AcceptedLedgerTx::pointer ();

    return it->second;
}

} //
