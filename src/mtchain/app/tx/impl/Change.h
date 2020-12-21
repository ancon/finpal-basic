//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_CHANGE_H_INCLUDED
#define MTCHAIN_TX_CHANGE_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/app/misc/AmendmentTable.h>
#include <mtchain/app/misc/NetworkOPs.h>
#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/Indexes.h>

namespace mtchain {

class Change
    : public Transactor
{
public:
    Change (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    TER doApply () override;
    void preCompute() override;

    static
    std::uint64_t
    calculateBaseFee (
        PreclaimContext const& ctx)
    {
        return 0;
    }

    static
    TER
    preclaim(PreclaimContext const &ctx);

private:
    TER applyAmendment ();

    TER applyFee ();
};

}

#endif
