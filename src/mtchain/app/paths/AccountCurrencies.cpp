//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/AccountCurrencies.h>

namespace mtchain {

hash_set<Currency> accountSourceCurrencies (
    AccountID const& account,
    std::shared_ptr<MTChainLineCache> const& lrCache,
    bool includeM)
{
    hash_set<Currency> currencies;

    // YYY Only bother if they are above reserve
    if (includeM)
        currencies.insert (mCurrency());

    // List of mtchain lines.
    auto& mtchainLines = lrCache->getMTChainLines (account);

    for (auto const& item : mtchainLines)
    {
        auto rspEntry = (MTChainState*) item.get ();
        assert (rspEntry);
        if (!rspEntry)
            continue;

        auto& saBalance = rspEntry->getBalance ();

        // Filter out non
        if (saBalance > zero
            // Have IOUs to send.
            || (rspEntry->getLimitPeer ()
                // Peer extends credit.
                && ((-saBalance) < rspEntry->getLimitPeer ()))) // Credit left.
        {
            currencies.insert (saBalance.getCurrency ());
        }
    }

    currencies.erase (badCurrency());
    return currencies;
}

hash_set<Currency> accountDestCurrencies (
    AccountID const& account,
    std::shared_ptr<MTChainLineCache> const& lrCache,
    bool includeM)
{
    hash_set<Currency> currencies;

    if (includeM)
        currencies.insert (mCurrency());
    // Even if account doesn't exist

    // List of mtchain lines.
    auto& mtchainLines = lrCache->getMTChainLines (account);

    for (auto const& item : mtchainLines)
    {
        auto rspEntry = (MTChainState*) item.get ();
        assert (rspEntry);
        if (!rspEntry)
            continue;

        auto& saBalance  = rspEntry->getBalance ();

        if (saBalance < rspEntry->getLimit ())                  // Can take more
            currencies.insert (saBalance.getCurrency ());
    }

    currencies.erase (badCurrency());
    return currencies;
}

} //
