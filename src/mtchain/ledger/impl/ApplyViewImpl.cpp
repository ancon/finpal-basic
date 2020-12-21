//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/ledger/ApplyViewImpl.h>
#include <mtchain/basics/contract.h>
#include <cassert>

namespace mtchain {

ApplyViewImpl::ApplyViewImpl(
    ReadView const* base, ApplyFlags flags)
    : ApplyViewBase (base, flags)
{
}

void
ApplyViewImpl::apply (OpenView& to,
    STTx const& tx, TER ter,
        beast::Journal j)
{
    items_.apply(to, tx, ter, deliver_, j);
}

std::size_t
ApplyViewImpl::size ()
{
    return items_.size ();
}

void
ApplyViewImpl::visit (
    OpenView& to,
    std::function <void (
        uint256 const& key,
        bool isDelete,
        std::shared_ptr <SLE const> const& before,
        std::shared_ptr <SLE const> const& after)> const& func)
{
    items_.visit (to, func);
}

} //
