//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_APP_PATHS_CURSOR_MTCHAINLIQUIDITY_H_INCLUDED
#define MTCHAIN_APP_PATHS_CURSOR_MTCHAINLIQUIDITY_H_INCLUDED

#include <mtchain/app/paths/cursor/PathCursor.h>
#include <mtchain/app/paths/MtchainCalc.h>
#include <mtchain/app/paths/Tuning.h>
#include <mtchain/ledger/View.h>
#include <mtchain/protocol/Rate.h>

namespace mtchain {
namespace path {

void mtchainLiquidity (
    MtchainCalc&,
    Rate const& qualityIn,
    Rate const& qualityOut,
    STAmount const& saPrvReq,
    STAmount const& saCurReq,
    STAmount& saPrvAct,
    STAmount& saCurAct,
    std::uint64_t& uRateMax);

Rate
quality_in (
    ReadView const& view,
    AccountID const& uToAccountID,
    AccountID const& uFromAccountID,
    Currency const& currency);

Rate
quality_out (
    ReadView const& view,
    AccountID const& uToAccountID,
    AccountID const& uFromAccountID,
    Currency const& currency);

} // path
} //

#endif
