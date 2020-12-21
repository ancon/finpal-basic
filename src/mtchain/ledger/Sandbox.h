//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_LEDGER_SANDBOX_H_INCLUDED
#define MTCHAIN_LEDGER_SANDBOX_H_INCLUDED

#include <mtchain/ledger/RawView.h>
#include <mtchain/ledger/detail/ApplyViewBase.h>

namespace mtchain {

/** Discardable, editable view to a ledger.

    The sandbox inherits the flags of the base.

    @note Presented as ApplyView to clients.
*/
class Sandbox
    : public detail::ApplyViewBase
{
public:
    Sandbox() = delete;
    Sandbox (Sandbox const&) = delete;
    Sandbox& operator= (Sandbox&&) = delete;
    Sandbox& operator= (Sandbox const&) = delete;

    Sandbox (Sandbox&&) = default;

    Sandbox (ReadView const* base, ApplyFlags flags)
        : ApplyViewBase (base, flags)
    {
    }

    Sandbox (ApplyView const* base)
        : Sandbox(base, base->flags())
    {
    }

    void
    apply (RawView& to)
    {
        items_.apply(to);
    }
};

} //

#endif
