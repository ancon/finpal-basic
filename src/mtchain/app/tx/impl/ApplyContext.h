//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_APPLYCONTEXT_H_INCLUDED
#define MTCHAIN_TX_APPLYCONTEXT_H_INCLUDED

#include <mtchain/app/main/Application.h>
#include <mtchain/ledger/ApplyViewImpl.h>
#include <mtchain/core/Config.h>
#include <mtchain/protocol/STTx.h>
#include <mtchain/protocol/MAmount.h>
#include <mtchain/beast/utility/Journal.h>
#include <boost/optional.hpp>
#include <utility>

namespace mtchain {

// tx_enable_test

/** State information when applying a tx. */
class ApplyContext
{
public:
    explicit
    ApplyContext (Application& app, OpenView& base,
        STTx const& tx, TER preclaimResult,
            std::uint64_t baseFee, ApplyFlags flags,
                    beast::Journal = {});

    Application& app;
    STTx const& tx;
    TER const preclaimResult;
    std::uint64_t const baseFee;
    beast::Journal const journal;

    ApplyView&
    view()
    {
        return *view_;
    }

    ApplyView const&
    view() const
    {
        return *view_;
    }

    // VFALCO Unfortunately this is necessary
    RawView&
    rawView()
    {
        return *view_;
    }

    /** Sets the DeliveredAmount field in the metadata */
    void
    deliver (STAmount const& amount)
    {
        view_->deliver(amount);
    }

    /** Discard changes and start fresh. */
    void
    discard();

    /** Apply the transaction result to the base. */
    void
    apply (TER);

    /** Get the number of unapplied changes. */
    std::size_t
    size ();

    /** Visit unapplied changes. */
    void
    visit (std::function <void (
        uint256 const& key,
        bool isDelete,
        std::shared_ptr <SLE const> const& before,
        std::shared_ptr <SLE const> const& after)> const& func);

    void
    destroyM (MAmount const& fee)
    {
        view_->rawDestroyM(fee);
    }

private:
    OpenView& base_;
    ApplyFlags flags_;
    boost::optional<ApplyViewImpl> view_;
};

} //

#endif
