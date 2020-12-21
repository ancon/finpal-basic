//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_SETREGULARKEY_H_INCLUDED
#define MTCHAIN_TX_SETREGULARKEY_H_INCLUDED

#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/TxFlags.h>
#include <mtchain/protocol/types.h>

namespace mtchain {

class SetRegularKey
    : public Transactor
{
public:
    SetRegularKey (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    bool
    affectsSubsequentTransactionAuth(STTx const& tx)
    {
        return true;
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    static
    std::uint64_t
    calculateBaseFee (
        PreclaimContext const& ctx);

    TER doApply () override;
};

} //

#endif

