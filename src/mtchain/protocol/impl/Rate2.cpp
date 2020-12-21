//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/Quality.h>
#include <mtchain/protocol/Rate.h>

namespace mtchain {

Rate const parityRate (QUALITY_ONE);

namespace detail {

STAmount as_amount (Rate const& rate)
{
    return { noIssue(), rate.value, -9, false };
}

}

STAmount
multiply (
    STAmount const& amount,
    Rate const& rate)
{
    assert (rate.value != 0);

    if (rate == parityRate)
        return amount;

    return multiply (
        amount,
        detail::as_amount(rate),
        amount.issue());
}

STAmount
multiplyRound (
    STAmount const& amount,
    Rate const& rate,
    bool roundUp)
{
    assert (rate.value != 0);

    if (rate == parityRate)
        return amount;

    return mulRound (
        amount,
        detail::as_amount(rate),
        amount.issue(),
        roundUp);
}

STAmount
multiplyRound (
    STAmount const& amount,
    Rate const& rate,
    Issue const& issue,
    bool roundUp)
{
    assert (rate.value != 0);

    if (rate == parityRate)
    {
        return amount;
    }

    return mulRound (
        amount,
        detail::as_amount(rate),
        issue,
        roundUp);
}

STAmount
divide (
    STAmount const& amount,
    Rate const& rate)
{
    assert (rate.value != 0);

    if (rate == parityRate)
        return amount;

    return divide (
        amount,
        detail::as_amount(rate),
        amount.issue());
}

STAmount
divideRound (
    STAmount const& amount,
    Rate const& rate,
    bool roundUp)
{
    assert (rate.value != 0);

    if (rate == parityRate)
        return amount;

    return divRound (
        amount,
        detail::as_amount(rate),
        amount.issue(),
        roundUp);
}

STAmount
divideRound (
    STAmount const& amount,
    Rate const& rate,
    Issue const& issue,
    bool roundUp)
{
    assert (rate.value != 0);

    if (rate == parityRate)
        return amount;

    return divRound (
        amount,
        detail::as_amount(rate),
        issue,
        roundUp);
}

}
