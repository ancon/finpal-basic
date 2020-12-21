//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_PAYCHAN_H_INCLUDED
#define MTCHAIN_TX_PAYCHAN_H_INCLUDED

#include <mtchain/app/tx/impl/Transactor.h>

namespace mtchain {

class PayChanCreate
    : public Transactor
{
public:
    explicit
    PayChanCreate (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    static
    TER
    preclaim(PreclaimContext const &ctx);

    TER
    doApply() override;
};

//------------------------------------------------------------------------------

class PayChanFund
    : public Transactor
{
public:
    explicit
    PayChanFund (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    TER
    doApply() override;
};

//------------------------------------------------------------------------------

class PayChanClaim
    : public Transactor
{
public:
    explicit
    PayChanClaim (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    TER
    doApply() override;
};

}  //

#endif
