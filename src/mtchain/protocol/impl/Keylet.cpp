//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/protocol/Keylet.h>
#include <mtchain/protocol/STLedgerEntry.h>

namespace mtchain {

bool
Keylet::check (SLE const& sle) const
{
    if (type == ltANY)
        return true;
    if (type == ltINVALID)
        return false;
    if (type == ltCHILD)
    {
        assert(sle.getType() != ltDIR_NODE);
        return sle.getType() != ltDIR_NODE;
    }
    assert(sle.getType() == type);
    return sle.getType() == type;
}

} //
