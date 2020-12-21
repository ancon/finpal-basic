//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_SETTRUST_H_INCLUDED
#define MTCHAIN_TX_SETTRUST_H_INCLUDED

#include <mtchain/protocol/Quality.h>
#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/TxFlags.h>

namespace mtchain {

class SetTrust
    : public Transactor
{
public:
    SetTrust (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    static
    TER
    preclaim(PreclaimContext const& ctx);

    TER doApply () override;
};

} //

#endif

