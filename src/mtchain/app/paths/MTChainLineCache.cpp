//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/MTChainLineCache.h>
#include <mtchain/ledger/OpenView.h>

namespace mtchain {

MTChainLineCache::MTChainLineCache(
    std::shared_ptr <ReadView const> const& ledger)
{
    // We want the caching that OpenView provides
    // And we need to own a shared_ptr to the input view
    // VFALCO TODO This should be a CachedLedger
    mLedger = std::make_shared<OpenView>(&*ledger, ledger);
}

std::vector<MTChainState::pointer> const&
MTChainLineCache::getMTChainLines (AccountID const& accountID)
{
    AccountKey key (accountID, hasher_ (accountID));

    std::lock_guard <std::mutex> sl (mLock);

    auto it = lines_.emplace (key,
        std::vector<MTChainState::pointer>());

    if (it.second)
        it.first->second = getMTChainStateItems (
            accountID, *mLedger);

    return it.first->second;
}

} //
