//------------------------------------------------------------------------------
/*
    This file is part of FinPald: https://github.com/finpal/finpal-basic
    Copyright (c) 2019 ~ 2020 FinPal Alliance.

    Permission to use, copy, modify, and/or distribute this software for any

*/
//==============================================================================

#ifndef MTCHAIN_TX_CREATEOFFER_H_INCLUDED
#define MTCHAIN_TX_CREATEOFFER_H_INCLUDED

#include <mtchain/app/ledger/Ledger.h>
#include <mtchain/app/tx/impl/OfferStream.h>
#include <mtchain/app/tx/impl/Taker.h>
#include <mtchain/app/tx/impl/Transactor.h>
#include <mtchain/basics/chrono.h>
#include <mtchain/basics/Log.h>
#include <mtchain/json/to_string.h>
#include <mtchain/protocol/Quality.h>
#include <mtchain/beast/utility/Journal.h>
#include <mtchain/beast/utility/WrappedSink.h>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mtchain {

class CreateOffer
    : public Transactor
{
public:
    CreateOffer (ApplyContext& ctx)
        : Transactor(ctx)
        , stepCounter_ (1000, j_)
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

    void
    preCompute() override;

    std::pair<TER, bool>
    applyGuts (ApplyView& view, ApplyView& view_cancel);

    TER
    doApply() override;

private:
    /** Determine if we are authorized to hold the asset we want to get */
    static
    TER
    checkAcceptAsset(ReadView const& view,
        ApplyFlags const flags, AccountID const id,
            beast::Journal const j, Issue const& issue);

    bool
    dry_offer (ApplyView& view, Offer const& offer);

    static
    std::pair<bool, Quality>
    select_path (
        bool have_direct, OfferStream const& direct,
        bool have_bridge, OfferStream const& leg1, OfferStream const& leg2);

    std::pair<TER, Amounts>
    bridged_cross (
        Taker& taker,
        ApplyView& view,
        ApplyView& view_cancel,
        NetClock::time_point const when);

    std::pair<TER, Amounts>
    direct_cross (
        Taker& taker,
        ApplyView& view,
        ApplyView& view_cancel,
        NetClock::time_point const when);

    // Step through the stream for as long as possible, skipping any offers
    // that are from the taker or which cross the taker's threshold.
    // Return false if the is no offer in the book, true otherwise.
    static
    bool
    step_account (OfferStream& stream, Taker const& taker);

    // True if the number of offers that have been crossed
    // exceeds the limit.
    bool
    reachedOfferCrossingLimit (Taker const& taker) const;

    // Fill offer as much as possible by consuming offers already on the books,
    // and adjusting account balances accordingly.
    //
    // Charges fees on top to taker.
    std::pair<TER, Amounts>
    cross (
        ApplyView& view,
        ApplyView& cancel_view,
        Amounts const& taker_amount);

    static
    std::string
    format_amount (STAmount const& amount);

private:
    // What kind of offer we are placing
    CrossType cross_type_;

    // The number of steps to take through order books while crossing
    OfferStream::StepCounter stepCounter_;

    // charge transaction fee for iou
    STAmount fee_;
    STAmount totalFee_;
};

}

#endif
