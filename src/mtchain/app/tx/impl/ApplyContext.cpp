//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/app/tx/impl/ApplyContext.h>
#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/Log.h>
#include <mtchain/json/to_string.h>
#include <mtchain/protocol/Indexes.h>
#include <cassert>

namespace mtchain {

ApplyContext::ApplyContext(Application& app_,
    OpenView& base, STTx const& tx_, TER preclaimResult_,
        std::uint64_t baseFee_, ApplyFlags flags,
            beast::Journal journal_)
    : app(app_)
    , tx(tx_)
    , preclaimResult(preclaimResult_)
    , baseFee(baseFee_)
    , journal(journal_)
    , base_ (base)
    , flags_(flags)
{
    view_.emplace(&base_, flags_);
}

void
ApplyContext::discard()
{
    view_.emplace(&base_, flags_);
}

void
ApplyContext::apply(TER ter)
{
    view_->apply(base_, tx, ter, journal);
}

std::size_t
ApplyContext::size()
{
    return view_->size();
}

void
ApplyContext::visit (std::function <void (
    uint256 const&, bool,
    std::shared_ptr<SLE const> const&,
    std::shared_ptr<SLE const> const&)> const& func)
{
    view_->visit(base_, func);
}

} //
