//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_ESCROW_H_INCLUDED
#define MTCHAIN_TX_ESCROW_H_INCLUDED

#include <mtchain/app/tx/impl/Transactor.h>

namespace mtchain {

class EscrowCreate
    : public Transactor
{
public:
    explicit
    EscrowCreate (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    MAmount
    calculateMaxSpend(STTx const& tx);

    static
    TER
    preflight (PreflightContext const& ctx);

    TER
    doApply() override;
};

//------------------------------------------------------------------------------

class EscrowFinish
    : public Transactor
{
public:
    explicit
    EscrowFinish (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    static
    std::uint64_t
    calculateBaseFee (PreclaimContext const& ctx);

    TER
    doApply() override;
};

//------------------------------------------------------------------------------

class EscrowCancel
    : public Transactor
{
public:
    explicit
    EscrowCancel (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    TER
    doApply() override;
};

} //

#endif
