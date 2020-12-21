//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_CREATETICKET_H_INCLUDED
#define MTCHAIN_TX_CREATETICKET_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/Log.h>
#include <mtchain/protocol/Indexes.h>

namespace mtchain {

class CreateTicket
    : public Transactor
{
public:
    CreateTicket (ApplyContext& ctx)
        : Transactor(ctx)
    {
    }

    static
    TER
    preflight (PreflightContext const& ctx);

    TER doApply () override;
};

}

#endif
