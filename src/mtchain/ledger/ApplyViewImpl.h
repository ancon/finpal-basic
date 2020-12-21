//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_LEDGER_APPLYVIEWIMPL_H_INCLUDED
#define MTCHAIN_LEDGER_APPLYVIEWIMPL_H_INCLUDED

#include <mtchain/ledger/OpenView.h>
#include <mtchain/ledger/detail/ApplyViewBase.h>
#include <mtchain/protocol/STAmount.h>
#include <mtchain/protocol/TER.h>
#include <boost/optional.hpp>

namespace mtchain {

/** Editable, discardable view that can build metadata for one tx.

    Iteration of the tx map is delegated to the base.

    @note Presented as ApplyView to clients.
*/
class ApplyViewImpl final
    : public detail::ApplyViewBase
{
public:
    ApplyViewImpl() = delete;
    ApplyViewImpl (ApplyViewImpl const&) = delete;
    ApplyViewImpl& operator= (ApplyViewImpl&&) = delete;
    ApplyViewImpl& operator= (ApplyViewImpl const&) = delete;

    ApplyViewImpl (ApplyViewImpl&&) = default;
    ApplyViewImpl(
        ReadView const* base, ApplyFlags flags);

    /** Apply the transaction.

        After a call to `apply`, the only valid
        operation on this object is to call the
        destructor.
    */
    void
    apply (OpenView& to,
        STTx const& tx, TER ter,
            beast::Journal j);

    /** Set the amount of currency delivered.

        This value is used when generating metadata
        for payments, to set the DeliveredAmount field.
        If the amount is not specified, the field is
        excluded from the resulting metadata.
    */
    void
    deliver (STAmount const& amount)
    {
        deliver_ = amount;
    }

    /** Get the number of modified entries
    */
    std::size_t
    size ();

    /** Visit modified entries
    */
    void
    visit (
        OpenView& target,
        std::function <void (
            uint256 const& key,
            bool isDelete,
            std::shared_ptr <SLE const> const& before,
            std::shared_ptr <SLE const> const& after)> const& func);
private:
    boost::optional<STAmount> deliver_;
};

} //

#endif
