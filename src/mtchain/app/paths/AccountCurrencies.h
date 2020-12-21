//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_ACCOUNTCURRENCIES_H_INCLUDED
#define MTCHAIN_APP_PATHS_ACCOUNTCURRENCIES_H_INCLUDED

#include <mtchain/app/paths/MTChainLineCache.h>
#include <mtchain/protocol/types.h>

namespace mtchain {

hash_set<Currency>
accountDestCurrencies(
    AccountID const& account,
        std::shared_ptr<MTChainLineCache> const& cache,
            bool includeM);

hash_set<Currency>
accountSourceCurrencies(
    AccountID const& account,
        std::shared_ptr<MTChainLineCache> const& lrLedger,
             bool includeM);

} //

#endif
