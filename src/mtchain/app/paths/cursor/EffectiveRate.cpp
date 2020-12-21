//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/paths/cursor/EffectiveRate.h>
#include <mtchain/basics/contract.h>

namespace mtchain {
namespace path {

Rate
effectiveRate(
    Issue const& issue,
    AccountID const& account1,
    AccountID const& account2,
    boost::optional<Rate> const& rate)
{
    // 1:1 transfer rate for M
    if (isM (issue))
        return parityRate;

    if (!rate)
        LogicError ("No transfer rate set for node.");

    // 1:1 transfer rate if either of the accounts is the issuer
    if (issue.account == account1 || issue.account == account2)
        return parityRate;

    return rate.get();
}

} // path
} //
