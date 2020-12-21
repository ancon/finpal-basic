//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_PAYMENT_H_INCLUDED
#define MTCHAIN_TX_PAYMENT_H_INCLUDED

#include <mtchain/app/paths/MtchainCalc.h>
#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/TxFlags.h>

namespace mtchain {

// See https://mtchain.io/wiki/Transaction_Format#Payment_.280.29

class Payment
    : public Transactor
{
    /* The largest number of paths we allow */
    static std::size_t const MaxPathSize = 6;

    /* The longest path we allow */
    static std::size_t const MaxPathLength = 8;

public:
    Payment (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    MAmount
    calculateMaxSpend(STTx const& tx);

    static
    TER
    preflight (PreflightContext const& ctx);

    static
    TER
    preclaim(PreclaimContext const& ctx);

    TER doApply () override;

    /* it will be called after doApply */
    void preApply (TER terResult) override;

    static
    void
    afterApply(ReadView const& view, STTx const& tx, TER terResult,
               Application& app, beast::Journal& j);

private:
    static
    TER
    preflight(PreflightContext const& ctx, AccountID const& uDstAccountID, STAmount const& saDstAmount);

    static
    TER
    preclaim(PreclaimContext const& ctx, AccountID const& uDstAccountID, STAmount const& saDstAmount);

    TER doApply (AccountID const& uDstAccountID, STAmount const& saDstAmount, STObject &obj);
};

} //

#endif
