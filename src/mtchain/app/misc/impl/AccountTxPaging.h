//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_MISC_IMPL_ACCOUNTTXPAGING_H_INCLUDED
#define MTCHAIN_APP_MISC_IMPL_ACCOUNTTXPAGING_H_INCLUDED

#include <mtchain/core/DatabaseCon.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <cstdint>
#include <string>
#include <utility>


//------------------------------------------------------------------------------

namespace mtchain {

void
convertBlobsToTxResult (
    NetworkOPs::AccountTxs& to,
    std::uint32_t ledger_index,
    std::string const& status,
    Blob const& rawTxn,
    Blob const& rawMeta,
    Application& app);

void
saveLedgerAsync (Application& app, std::uint32_t seq);

void
accountTxPage (
    DatabaseCon& database,
    AccountIDCache const& idCache,
    std::function<void (std::uint32_t)> const& onUnsavedLedger,
    std::function<void (std::uint32_t,
                        std::string const&,
                        Blob const&,
                        Blob const&)> const&,
    AccountID const& account,
    std::int32_t minLedger,
    std::int32_t maxLedger,
    bool forward,
    Json::Value& token,
    int limit,
    bool bAdmin,
    std::uint32_t pageLength);

}

#endif
