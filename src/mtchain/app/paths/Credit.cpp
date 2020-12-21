//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/ledger/ReadView.h>
#include <mtchain/protocol/AmountConversions.h>
#include <mtchain/protocol/STAmount.h>
#include <mtchain/protocol/Indexes.h>

namespace mtchain {

STAmount
creditLimit (
    ReadView const& view,
    AccountID const& account,
    AccountID const& issuer,
    Currency const& currency)
{
    STAmount result ({currency, account});

    auto sleMtchainState = view.read(
        keylet::line(account, issuer, currency));

    if (sleMtchainState)
    {
        result = sleMtchainState->getFieldAmount (
            account < issuer ? sfLowLimit : sfHighLimit);
        result.setIssuer (account);
    }

    assert (result.getIssuer () == account);
    assert (result.getCurrency () == currency);
    return result;
}

IOUAmount
creditLimit2 (
    ReadView const& v,
    AccountID const& acc,
    AccountID const& iss,
    Currency const& cur)
{
    return toAmount<IOUAmount> (creditLimit (v, acc, iss, cur));
}

STAmount creditBalance (
    ReadView const& view,
    AccountID const& account,
    AccountID const& issuer,
    Currency const& currency)
{
    STAmount result ({currency, account});

    auto sleMtchainState = view.read(
        keylet::line(account, issuer, currency));

    if (sleMtchainState)
    {
        result = sleMtchainState->getFieldAmount (sfBalance);
        if (account < issuer)
            result.negate ();
        result.setIssuer (account);
    }

    assert (result.getIssuer () == account);
    assert (result.getCurrency () == currency);
    return result;
}

} //
