//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_SETACCOUNT_H_INCLUDED
#define MTCHAIN_TX_SETACCOUNT_H_INCLUDED

#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/Log.h>
#include <mtchain/core/Config.h>
#include <mtchain/protocol/Indexes.h>
#include <mtchain/protocol/Quality.h>
#include <mtchain/protocol/TxFlags.h>

namespace mtchain {

class SetAccount
    : public Transactor
{
    static std::size_t const DOMAIN_BYTES_MAX = 256;

    TER setIOUPrecision (std::shared_ptr<SLE> const& sleAccount);

public:
    SetAccount (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    bool
    affectsSubsequentTransactionAuth(STTx const& tx);

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
