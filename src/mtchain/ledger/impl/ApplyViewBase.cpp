//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#include <BeastConfig.h>
#include <mtchain/ledger/detail/ApplyViewBase.h>
#include <mtchain/basics/contract.h>
#include <mtchain/ledger/View.h>

namespace mtchain {
namespace detail {

ApplyViewBase::ApplyViewBase(
    ReadView const* base, ApplyFlags flags)
    : flags_ (flags)
    , base_ (base)
{
}

//---

bool
ApplyViewBase::open() const
{
    return base_->open();
}

LedgerInfo const&
ApplyViewBase::info() const
{
    return base_->info();
}

Fees const&
ApplyViewBase::fees() const
{
    return base_->fees();
}

Rules const&
ApplyViewBase::rules() const
{
    return base_->rules();
}

bool
ApplyViewBase::exists (Keylet const& k) const
{
    return items_.exists(*base_, k);
}

auto
ApplyViewBase::succ (key_type const& key,
    boost::optional<key_type> const& last) const ->
        boost::optional<key_type>
{
    return items_.succ(*base_, key, last);
}

std::shared_ptr<SLE const>
ApplyViewBase::read (Keylet const& k) const
{
    return items_.read(*base_, k);
}

auto
ApplyViewBase::slesBegin() const ->
    std::unique_ptr<sles_type::iter_base>
{
    return base_->slesBegin();
}

auto
ApplyViewBase::slesEnd() const ->
    std::unique_ptr<sles_type::iter_base>
{
    return base_->slesEnd();
}

auto
ApplyViewBase::slesUpperBound(uint256 const& key) const ->
    std::unique_ptr<sles_type::iter_base>
{
    return base_->slesUpperBound(key);
}

auto
ApplyViewBase::txsBegin() const ->
    std::unique_ptr<txs_type::iter_base>
{
    return base_->txsBegin();
}

auto
ApplyViewBase::txsEnd() const ->
    std::unique_ptr<txs_type::iter_base>
{
    return base_->txsEnd();
}

bool
ApplyViewBase::txExists (key_type const& key) const
{
    return base_->txExists(key);
}

auto
ApplyViewBase::txRead(
    key_type const& key) const ->
        tx_type
{
    return base_->txRead(key);
}

//---

ApplyFlags
ApplyViewBase::flags() const
{
    return flags_;
}

std::shared_ptr<SLE>
ApplyViewBase::peek (Keylet const& k)
{
    return items_.peek(*base_, k);
}

void
ApplyViewBase::erase(
    std::shared_ptr<SLE> const& sle)
{
    items_.erase(*base_, sle);
}

void
ApplyViewBase::insert(
    std::shared_ptr<SLE> const& sle)
{
    items_.insert(*base_, sle);
}

void
ApplyViewBase::update(
    std::shared_ptr<SLE> const& sle)
{
    items_.update(*base_, sle);
}

//---

void
ApplyViewBase::rawErase(
    std::shared_ptr<SLE> const& sle)
{
    items_.rawErase(*base_, sle);
}

void
ApplyViewBase::rawInsert(
    std::shared_ptr<SLE> const& sle)
{
    items_.insert(*base_, sle);
}

void
ApplyViewBase::rawReplace(
    std::shared_ptr<SLE> const& sle)
{
    items_.replace(*base_, sle);
}

void
ApplyViewBase::rawDestroyM(
    MAmount const& fee)
{
    items_.destroyM(fee);
}

void
ApplyViewBase::creditHook (AccountID const& from,  AccountID const& to,
                           STAmount const& amount, STAmount const& preCreditBalance)
{
    assert (amount.signum() > 0);
    STAmount balance = amount;
    switch (preCreditBalance.signum())
    {
    case -1: // sender is issuer
        assert (from == balance.getIssuer());
        break;
    case  1: // receiver is issuer
        assert (to == balance.getIssuer());
        balance.negate();
        break;
    case  0: // sender is issuer
        if (from != balance.getIssuer())
        {
            balance.setIssuer(from);
            balance = balance.zeroed();
        }
        break;
    default:
        assert (false);
    }

    updateIssueTotalAmount(balance, *this, debugLog());
}

} // detail
} //
